#include "RenderingHandler.hpp"

#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

RenderingHandler::RenderingHandler(ECSCore* pECS, Device* pDevice)
    :m_pECS(pECS),
    m_pDevice(pDevice),
    m_pMeshRenderer(new MeshRenderer(pECS, pDevice, this)),
    m_pUIRenderer(new UIRenderer(pECS, pDevice, this))
{
    std::fill_n(m_ppCommandPools, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppCommandLists, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppRenderingSemaphores, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppPrimaryBufferFences, MAX_FRAMES_IN_FLIGHT, nullptr);
}

RenderingHandler::~RenderingHandler()
{
    delete m_pMeshRenderer;
    delete m_pUIRenderer;

    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppCommandPools[frameIndex];
        delete m_ppCommandLists[frameIndex];
        delete m_ppRenderingSemaphores[frameIndex];
        delete m_ppPrimaryBufferFences[frameIndex];
    }
}

bool RenderingHandler::init()
{
    m_Renderers = {
        m_pMeshRenderer,
        m_pUIRenderer
    };

    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        m_ppCommandPools[frameIndex] = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
        if (!m_ppCommandPools[frameIndex]) {
            return false;
        }

        m_ppCommandPools[frameIndex]->allocateCommandLists(&m_ppCommandLists[frameIndex], 1u, COMMAND_LIST_LEVEL::PRIMARY);
        if (!m_ppCommandLists[frameIndex]) {
            return false;
        }

        m_ppRenderingSemaphores[frameIndex] = m_pDevice->createSemaphore();
        if (!m_ppRenderingSemaphores[frameIndex]) {
            return false;
        }

        m_ppPrimaryBufferFences[frameIndex] = m_pDevice->createFence(true);
        if (!m_ppPrimaryBufferFences[frameIndex]) {
            return false;
        }
    }

    return true;
}

void RenderingHandler::render()
{
    beginFrame();

    updateBuffers();
    recordSecondaryCommandBuffers();
    recordPrimaryCommandBuffer();

    endFrame();
}

void RenderingHandler::waitAllFrames()
{
    // Calling waitForFences with waitAll is dangerous as the goal is to always have a frame being rendered.
    // Instead, wait for each fence one by one.
    // Start by waiting for the upcoming frame's fence. It's the most likely to be signaled first
    uint32_t frameIndex = (m_pDevice->getFrameIndex() + 1u) % MAX_FRAMES_IN_FLIGHT;
    for (uint32_t waitedFrameCount = 0u; waitedFrameCount < MAX_FRAMES_IN_FLIGHT; waitedFrameCount += 1u) {
        m_pDevice->waitForFences(&m_ppPrimaryBufferFences[frameIndex], 1u, false, UINT64_MAX);
        frameIndex = (frameIndex + 1u) % MAX_FRAMES_IN_FLIGHT;
    }
}

void RenderingHandler::beginFrame()
{
    Swapchain* pSwapchain = m_pDevice->getSwapchain();
    uint32_t& frameIndex = m_pDevice->getFrameIndex();
    pSwapchain->acquireNextBackbuffer(frameIndex, SYNC_OPTION::SEMAPHORE);

    m_pDevice->waitForFences(&m_ppPrimaryBufferFences[frameIndex], 1u, false, UINT64_MAX);
    m_ppPrimaryBufferFences[frameIndex]->reset();

    m_ppCommandLists[frameIndex]->begin({}, nullptr);
}

void RenderingHandler::endFrame()
{
    const uint32_t frameIndex = m_pDevice->getFrameIndex();
    ISemaphore* pBackbufferReadySemaphore = m_pDevice->getSwapchain()->getCurrentSemaphore();

    PIPELINE_STAGE waitStage = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;

    SemaphoreSubmitInfo semaphoreInfo = {};
    semaphoreInfo.ppWaitSemaphores      = &pBackbufferReadySemaphore;
    semaphoreInfo.WaitSemaphoreCount    = 1u;
    semaphoreInfo.pWaitStageFlags       = &waitStage;
    semaphoreInfo.ppSignalSemaphores    = &m_ppRenderingSemaphores[frameIndex];
    semaphoreInfo.SignalSemaphoreCount  = 1u;
    m_pDevice->graphicsQueueSubmit(m_ppCommandLists[frameIndex], m_ppPrimaryBufferFences[frameIndex], &semaphoreInfo);

    m_pDevice->getSwapchain()->present(&m_ppRenderingSemaphores[frameIndex], 1u);
}

void RenderingHandler::updateBuffers()
{
    std::vector<std::thread> threads;
    threads.reserve(m_Renderers.size());

    for (Renderer* pRenderer : m_Renderers) {
        threads.push_back(std::thread(&Renderer::updateBuffers, pRenderer));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

void RenderingHandler::recordSecondaryCommandBuffers()
{
    std::vector<std::thread> threads;
    threads.reserve(m_Renderers.size());

    for (Renderer* pRenderer : m_Renderers) {
        threads.push_back(std::thread(&Renderer::recordCommands, pRenderer));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

void RenderingHandler::recordPrimaryCommandBuffer()
{
    const uint32_t frameIndex = m_pDevice->getFrameIndex();
    ICommandList* pPrimaryCommandList = m_ppCommandLists[frameIndex];

    // Begin mesh render pass
    std::array<ClearValue, 2> pClearValues;
    std::fill_n(pClearValues[0].ClearColorValue.uint32, 4u, 0u);
    pClearValues[1].DepthStencilValue.Depth     = 1.0f;
    pClearValues[1].DepthStencilValue.Stencil   = 0u;

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_pMeshRenderer->getFramebuffer(frameIndex);
    renderPassBeginInfo.pClearValues        = pClearValues.data();
    renderPassBeginInfo.ClearValueCount     = (uint32_t)pClearValues.size();
    renderPassBeginInfo.RecordingListType   = COMMAND_LIST_LEVEL::SECONDARY;
    m_ppCommandLists[frameIndex]->beginRenderPass(m_pMeshRenderer->getRenderPass(), renderPassBeginInfo);

    m_pMeshRenderer->executeCommands(pPrimaryCommandList);

    pPrimaryCommandList->endRenderPass();

    // Begin UI render pass
    renderPassBeginInfo.pFramebuffer    = m_pUIRenderer->getFramebuffer(frameIndex);
    renderPassBeginInfo.ClearValueCount = 0u;
    m_ppCommandLists[frameIndex]->beginRenderPass(m_pUIRenderer->getRenderPass(), renderPassBeginInfo);

    m_pUIRenderer->executeCommands(pPrimaryCommandList);

    m_ppCommandLists[frameIndex]->endRenderPass();
    m_ppCommandLists[frameIndex]->end();
}

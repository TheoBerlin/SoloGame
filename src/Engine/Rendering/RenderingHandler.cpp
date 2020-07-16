#include "RenderingHandler.hpp"

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

RenderingHandler::RenderingHandler(ECSCore* pECS, Device* pDevice)
    :m_pECS(pECS),
    m_pDevice(pDevice),
    m_MeshRenderer(pECS, pDevice, this),
    m_UIRenderer(pECS, pDevice, this),
    m_pPrimaryBufferFence(nullptr)
{
    std::fill(&m_ppCommandPools[0u], &m_ppCommandPools[MAX_FRAMES_IN_FLIGHT], nullptr);
    std::fill(&m_ppCommandLists[0u], &m_ppCommandLists[MAX_FRAMES_IN_FLIGHT], nullptr);
    std::fill(&m_ppRenderingSemaphores[0u], &m_ppRenderingSemaphores[MAX_FRAMES_IN_FLIGHT], nullptr);
}

RenderingHandler::~RenderingHandler()
{
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppCommandPools[frameIndex];
        delete m_ppCommandLists[frameIndex];
        delete m_ppRenderingSemaphores[frameIndex];
    }

    delete m_pPrimaryBufferFence;
}

bool RenderingHandler::init()
{
    m_Renderers = {
        &m_MeshRenderer,
        &m_UIRenderer
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
    }

    m_pPrimaryBufferFence = m_pDevice->createFence(true);
    return m_pPrimaryBufferFence;
}

void RenderingHandler::render()
{
    beginFrame();

    updateBuffers();
    recordSecondaryCommandBuffers();
    recordPrimaryCommandBuffer();

    endFrame();
}

void RenderingHandler::beginFrame()
{
    Swapchain* pSwapchain = m_pDevice->getSwapchain();
    uint32_t& frameIndex = m_pDevice->getFrameIndex();
    pSwapchain->acquireNextBackbuffer(frameIndex, SYNC_OPTION::SEMAPHORE);

    m_pDevice->waitForFences(&m_pPrimaryBufferFence, 1u, false, UINT64_MAX);
    m_pPrimaryBufferFence->reset();

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
    m_pDevice->graphicsQueueSubmit(m_ppCommandLists[frameIndex], m_pPrimaryBufferFence, &semaphoreInfo);

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
    std::fill(pClearValues[0].ClearColorValue.uint32, &pClearValues[0].ClearColorValue.uint32[3], 0u);
    pClearValues[1].DepthStencilValue.Depth     = 1.0f;
    pClearValues[1].DepthStencilValue.Stencil   = 0u;

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_MeshRenderer.getFramebuffer(frameIndex);
    renderPassBeginInfo.pClearValues        = pClearValues.data();
    renderPassBeginInfo.ClearValueCount     = (uint32_t)pClearValues.size();
    renderPassBeginInfo.RecordingListType   = COMMAND_LIST_LEVEL::SECONDARY;
    m_ppCommandLists[frameIndex]->beginRenderPass(m_MeshRenderer.getRenderPass(), renderPassBeginInfo);

    m_MeshRenderer.executeCommands(pPrimaryCommandList);

    pPrimaryCommandList->endRenderPass();

    // Begin UI render pass
    renderPassBeginInfo.pFramebuffer    = m_UIRenderer.getFramebuffer(frameIndex);
    renderPassBeginInfo.ClearValueCount = 0u;
    m_ppCommandLists[frameIndex]->beginRenderPass(m_UIRenderer.getRenderPass(), renderPassBeginInfo);

    m_UIRenderer.executeCommands(pPrimaryCommandList);

    m_ppCommandLists[frameIndex]->endRenderPass();
    m_ppCommandLists[frameIndex]->end();
}

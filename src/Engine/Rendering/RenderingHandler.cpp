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
    m_pRenderPass(nullptr),
    m_pPrimaryBufferFence(nullptr)
{
    std::fill(&m_ppCommandPools[0u], &m_ppCommandPools[MAX_FRAMES_IN_FLIGHT], nullptr);
    std::fill(&m_ppCommandLists[0u], &m_ppCommandLists[MAX_FRAMES_IN_FLIGHT], nullptr);
    std::fill(&m_ppFramebuffersBackDepth[0u], &m_ppFramebuffersBackDepth[MAX_FRAMES_IN_FLIGHT], nullptr);
    std::fill(&m_ppRenderingSemaphores[0u], &m_ppRenderingSemaphores[MAX_FRAMES_IN_FLIGHT], nullptr);
}

RenderingHandler::~RenderingHandler()
{
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppCommandPools[frameIndex];
        delete m_ppCommandLists[frameIndex];
        delete m_ppFramebuffersBackDepth[frameIndex];
        delete m_ppRenderingSemaphores[frameIndex];
    }

    delete m_pPrimaryBufferFence;
    delete m_pRenderPass;
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

    if (!createRenderPass()) {
        return false;
    }

    if (!createFramebuffers()) {
        return false;
    }

    m_pPrimaryBufferFence = m_pDevice->createFence(false);
    return m_pPrimaryBufferFence;
}

void RenderingHandler::render()
{
    beginFrame();

    updateBuffers();
    recordCommandBuffers();
    executeCommandBuffers();

    endFrame();
}

bool RenderingHandler::createRenderPass()
{
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    Texture* pBackbuffer = m_pDevice->getBackbuffer(0u);
    AttachmentInfo backBufferAttachment   = {};
    backBufferAttachment.Format           = pBackbuffer->getFormat();
    backBufferAttachment.Samples          = 1u;
    backBufferAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    backBufferAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    backBufferAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    backBufferAttachment.FinalLayout      = TEXTURE_LAYOUT::PRESENT;

    Texture* pDepthStencil = m_pDevice->getDepthStencil(0u);
    AttachmentInfo depthStencilAttachment = {};
    depthStencilAttachment.Format           = pDepthStencil->getFormat();
    depthStencilAttachment.Samples          = 1u;
    depthStencilAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    depthStencilAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    depthStencilAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    depthStencilAttachment.FinalLayout      = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    // Mesh subpass
    SubpassInfo meshSubpass = {};
    AttachmentReference backbufferRef   = {};
    backbufferRef.AttachmentIndex       = 0;
    backbufferRef.Layout                = TEXTURE_LAYOUT::RENDER_TARGET;

    AttachmentReference depthStencilRef = {};
    depthStencilRef.AttachmentIndex     = 1;
    depthStencilRef.Layout              = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    meshSubpass.ColorAttachments        = { backbufferRef };
    meshSubpass.pDepthStencilAttachment = &depthStencilRef;
    meshSubpass.PipelineBindPoint       = PIPELINE_BIND_POINT::GRAPHICS;

    // Mesh subpass dependency
    SubpassDependency meshSubpassDependency = {};
    meshSubpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    meshSubpassDependency.DstSubpass        = 0;
    meshSubpassDependency.SrcStage          = PIPELINE_STAGE::BOTTOM_OF_PIPE;
    meshSubpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    meshSubpassDependency.SrcAccessMask     = RESOURCE_ACCESS::MEMORY_READ;
    meshSubpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    meshSubpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    // UI subpass
    SubpassInfo UISubpass = {};
    UISubpass.ColorAttachments        = { backbufferRef };
    UISubpass.pDepthStencilAttachment = &depthStencilRef;
    UISubpass.PipelineBindPoint       = PIPELINE_BIND_POINT::GRAPHICS;

    // UI subpass dependency
    SubpassDependency UISubpassDependency = {};
    UISubpassDependency.SrcSubpass        = 0u;
    UISubpassDependency.DstSubpass        = 1u;
    UISubpassDependency.SrcStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    UISubpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    UISubpassDependency.SrcAccessMask     = { };
    UISubpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    UISubpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    renderPassInfo.AttachmentInfos  = { backBufferAttachment, depthStencilAttachment };
    renderPassInfo.Subpasses        = { meshSubpass, UISubpass };
    renderPassInfo.Dependencies     = { meshSubpassDependency, UISubpassDependency };

    m_pRenderPass = m_pDevice->createRenderPass(renderPassInfo);
    return m_pRenderPass;
}

bool RenderingHandler::createFramebuffers()
{
    FramebufferInfo framebufferInfoBackDepth = {};
    framebufferInfoBackDepth.pRenderPass     = m_pRenderPass;
    framebufferInfoBackDepth.Dimensions      = m_pDevice->getBackbuffer(0u)->getDimensions();
    framebufferInfoBackDepth.Attachments.resize(2u);

    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        framebufferInfoBackDepth.Attachments[0u] = m_pDevice->getBackbuffer(frameIndex);
        framebufferInfoBackDepth.Attachments[1u] = m_pDevice->getDepthStencil(frameIndex);
        m_ppFramebuffersBackDepth[frameIndex] = m_pDevice->createFramebuffer(framebufferInfoBackDepth);
        if (!m_ppFramebuffersBackDepth[frameIndex]) {
            return false;
        }
    }

    return true;
}

void RenderingHandler::beginFrame()
{
    Swapchain* pSwapchain = m_pDevice->getSwapchain();
    uint32_t& frameIndex = m_pDevice->getFrameIndex();
    pSwapchain->acquireNextBackbuffer(frameIndex, SYNC_OPTION::SEMAPHORE);

    m_pDevice->waitForFences(&m_pPrimaryBufferFence, 1u, false, UINT64_MAX);
    m_pPrimaryBufferFence->reset();

    m_ppCommandLists[frameIndex]->begin({}, nullptr);

    // Begin render pass
    std::array<ClearValue, 2> pClearValues;
    std::fill(pClearValues[0].ClearColorValue.uint32, &pClearValues[0].ClearColorValue.uint32[3], 0u);
    pClearValues[1].DepthStencilValue.Depth     = 1.0f;
    pClearValues[1].DepthStencilValue.Stencil   = 0u;

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_ppFramebuffersBackDepth[frameIndex];
    renderPassBeginInfo.pClearValues        = pClearValues.data();
    renderPassBeginInfo.ClearValueCount     = (uint32_t)pClearValues.size();
    renderPassBeginInfo.RecordingListType   = COMMAND_LIST_LEVEL::SECONDARY;
    m_ppCommandLists[frameIndex]->beginRenderPass(m_pRenderPass, renderPassBeginInfo);
}

void RenderingHandler::endFrame()
{
    const uint32_t frameIndex = m_pDevice->getFrameIndex();
    m_ppCommandLists[frameIndex]->endRenderPass();
    m_ppCommandLists[frameIndex]->end();

    ISemaphore* pBackbufferReadySemaphore = m_pDevice->getSwapchain()->getSemaphore(frameIndex);

    SemaphoreSubmitInfo semaphoreInfo = {};
    semaphoreInfo.ppWaitSemaphores      = &pBackbufferReadySemaphore;
    semaphoreInfo.WaitSemaphoreCount    = 1u;
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

void RenderingHandler::recordCommandBuffers()
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

void RenderingHandler::executeCommandBuffers()
{
    ICommandList* pPrimaryCommandList = m_ppCommandLists[m_pDevice->getFrameIndex()];

    m_MeshRenderer.executeCommands(pPrimaryCommandList);
    pPrimaryCommandList->nextSubpass(COMMAND_LIST_LEVEL::SECONDARY);
    m_UIRenderer.executeCommands(pPrimaryCommandList);
}

#include "UIRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>

#include <algorithm>

UIRenderer::UIRenderer(Device* pDevice, RenderingHandler* pRenderingHandler)
    :Renderer(pDevice, pRenderingHandler),
    m_CommandListsToReset(MAX_FRAMES_IN_FLIGHT),
    m_pQuad(nullptr),
    m_pAniSampler(nullptr),
    m_pRenderPass(nullptr),
    m_pDescriptorSetLayout(nullptr),
    m_pPipelineLayout(nullptr),
    m_pPipeline(nullptr)
{
    std::fill_n(m_ppCommandLists, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppCommandPools, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppFramebuffers, MAX_FRAMES_IN_FLIGHT, nullptr);

    EntitySubscriberRegistration entitySubscriberRegistration = {
        .EntitySubscriptionRegistrations = {
            {
                .pSubscriber = &m_Panels,
                .ComponentAccesses =
                {
                    { R, UIPanelComponent::Type() }
                },
                .OnEntityAdded = std::bind_front(&UIRenderer::OnPanelAdded, this),
                .OnEntityRemoval = std::bind_front(&UIRenderer::OnPanelRemoved, this)
            }
        }
    };

    RegisterRenderer(entitySubscriberRegistration);
}

UIRenderer::~UIRenderer()
{
    // Delete all panel render resources
    for (Entity entity : m_Panels) {
        PanelRenderResources& panelRenderResources = m_PanelRenderResources.IndexID(entity);
        delete panelRenderResources.pBuffer;
        delete panelRenderResources.pDescriptorSet;

        m_PanelRenderResources.Pop(entity);
    }

    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppCommandLists[frameIndex];
        delete m_ppCommandPools[frameIndex];
        delete m_ppFramebuffers[frameIndex];
    }

    delete m_pDescriptorSetLayout;
    delete m_pRenderPass;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool UIRenderer::Init()
{
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        m_ppCommandPools[frameIndex] = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
        if (!m_ppCommandPools[frameIndex]) {
            return false;
        }

        m_ppCommandPools[frameIndex]->allocateCommandLists(&m_ppCommandLists[frameIndex], 1u, COMMAND_LIST_LEVEL::SECONDARY);
        if (!m_ppCommandLists[frameIndex]) {
            return false;
        }
    }

    ShaderResourceHandler* pShaderResourceHandler = ShaderResourceHandler::GetInstance();
    m_pQuad         = pShaderResourceHandler->GetQuarterScreenQuad();
    m_pAniSampler   = pShaderResourceHandler->GetAniSampler();

    if (!CreateDescriptorSetLayouts()) {
        return false;
    }

    if (!CreateRenderPass()) {
        return false;
    }

    if (!CreateFramebuffers()) {
        return false;
    }

    return CreatePipeline();
}

void UIRenderer::UpdateBuffers()
{
    constexpr const size_t bufferSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float);              // Highlight factor

    const ComponentArray<UIPanelComponent>* pPanelComponents = ECSCore::GetInstance()->GetComponentArray<UIPanelComponent>();

    for (const Entity& entity : m_Panels) {
        const UIPanelComponent& panel = pPanelComponents->GetConstData(entity);
        PanelRenderResources& panelRenderResources = m_PanelRenderResources.IndexID(entity);

        // Set per-object buffer
        void* pMappedBuffer = nullptr;
        m_pDevice->map(panelRenderResources.pBuffer, &pMappedBuffer);
        memcpy(pMappedBuffer, &panel, bufferSize);
        m_pDevice->unmap(panelRenderResources.pBuffer);
    }
}

void UIRenderer::RecordCommands()
{
    if (m_CommandListsToReset == 0u) {
        return;
    }

    m_CommandListsToReset -= 1u;

    const uint32_t frameIndex = m_pDevice->getFrameIndex();
    ICommandList* pCommandList = m_ppCommandLists[frameIndex];

    CommandListBeginInfo beginInfo = {};
    beginInfo.pRenderPass   = m_pRenderPass;
    beginInfo.Subpass       = 0u;
    beginInfo.pFramebuffer  = m_ppFramebuffers[frameIndex];
    pCommandList->begin(COMMAND_LIST_USAGE::WITHIN_RENDER_PASS, &beginInfo);

    if (!m_Panels.Empty()) {
        pCommandList->bindPipeline(m_pPipeline);
        pCommandList->bindVertexBuffer(0u, m_pQuad);

        for (const PanelRenderResources& panelRenderResources : m_PanelRenderResources) {
            pCommandList->bindDescriptorSet(panelRenderResources.pDescriptorSet, m_pPipelineLayout, 0u);
            pCommandList->draw(4u);
        }
    }

    pCommandList->end();
}

void UIRenderer::ExecuteCommands(ICommandList* pPrimaryCommandList)
{
    pPrimaryCommandList->executeSecondaryCommandList(m_ppCommandLists[m_pDevice->getFrameIndex()]);
}

bool UIRenderer::CreateDescriptorSetLayouts()
{
    m_pDescriptorSetLayout = m_pDevice->createDescriptorSetLayout();
    if (!m_pDescriptorSetLayout) {
        return false;
    }

    m_pDescriptorSetLayout->addBindingUniformBuffer(SHADER_BINDING::PER_OBJECT, SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayout->addBindingCombinedTextureSampler(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    return m_pDescriptorSetLayout->finalize(m_pDevice);
}

bool UIRenderer::CreateRenderPass()
{
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    Texture* pBackbuffer = m_pDevice->getBackbuffer(0u);
    AttachmentInfo backBufferAttachment   = {};
    backBufferAttachment.Format           = pBackbuffer->getFormat();
    backBufferAttachment.Samples          = 1u;
    backBufferAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    backBufferAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    backBufferAttachment.InitialLayout    = TEXTURE_LAYOUT::RENDER_TARGET;
    backBufferAttachment.FinalLayout      = TEXTURE_LAYOUT::PRESENT;

    Texture* pDepthStencil = m_pDevice->getDepthStencil(0u);
    AttachmentInfo depthStencilAttachment   = {};
    depthStencilAttachment.Format           = pDepthStencil->getFormat();
    depthStencilAttachment.Samples          = 1u;
    depthStencilAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    depthStencilAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    depthStencilAttachment.InitialLayout    = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;
    depthStencilAttachment.FinalLayout      = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    // Subpass
    SubpassInfo subpass = {};
    AttachmentReference backbufferRef   = {};
    backbufferRef.AttachmentIndex       = 0;
    backbufferRef.Layout                = TEXTURE_LAYOUT::RENDER_TARGET;

    AttachmentReference depthStencilRef = {};
    depthStencilRef.AttachmentIndex     = 1;
    depthStencilRef.Layout              = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    subpass.ColorAttachments        = { backbufferRef };
    subpass.pDepthStencilAttachment = &depthStencilRef;
    subpass.PipelineBindPoint       = PIPELINE_BIND_POINT::GRAPHICS;

    // Subpass dependency
    SubpassDependency subpassDependency = {};
    subpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    subpassDependency.DstSubpass        = 0;
    subpassDependency.SrcStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.SrcAccessMask     = { };
    subpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    renderPassInfo.AttachmentInfos  = { backBufferAttachment, depthStencilAttachment };
    renderPassInfo.Subpasses        = { subpass };
    renderPassInfo.Dependencies     = { subpassDependency };

    m_pRenderPass = m_pDevice->createRenderPass(renderPassInfo);
    return m_pRenderPass;
}

bool UIRenderer::CreateFramebuffers()
{
    FramebufferInfo framebufferInfo = {};
    framebufferInfo.pRenderPass     = m_pRenderPass;
    framebufferInfo.Dimensions      = m_pDevice->getBackbuffer(0u)->getDimensions();

    for (uint32_t framebufferIdx = 0u; framebufferIdx < MAX_FRAMES_IN_FLIGHT; framebufferIdx += 1u) {
        framebufferInfo.Attachments = { m_pDevice->getBackbuffer(framebufferIdx), m_pDevice->getDepthStencil(framebufferIdx) };
        m_ppFramebuffers[framebufferIdx] = m_pDevice->createFramebuffer(framebufferInfo);

        if (!m_ppFramebuffers[framebufferIdx]) {
            return false;
        }
    }

    return true;
}

bool UIRenderer::CreatePipeline()
{
    m_pPipelineLayout = m_pDevice->createPipelineLayout({ m_pDescriptorSetLayout });
    if (!m_pPipelineLayout) {
        return false;
    }

    PipelineInfo pipelineInfo = {};
    pipelineInfo.ShaderInfos = {
        {"UI", SHADER_TYPE::VERTEX_SHADER},
        {"UI", SHADER_TYPE::FRAGMENT_SHADER}
    };

    pipelineInfo.PrimitiveTopology = PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP;

    const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = (float)backbufferDims.x;
    viewport.Height   = (float)backbufferDims.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pipelineInfo.Viewports = { viewport };

    Rectangle2D scissorRectangle = {};
    scissorRectangle.Extent = backbufferDims;
    pipelineInfo.ScissorRectangles = { scissorRectangle };

    pipelineInfo.RasterizerStateInfo = {};
    pipelineInfo.RasterizerStateInfo.PolygonMode            = POLYGON_MODE::FILL;
    pipelineInfo.RasterizerStateInfo.CullMode               = CULL_MODE::NONE;
    pipelineInfo.RasterizerStateInfo.FrontFaceOrientation   = FRONT_FACE_ORIENTATION::CLOCKWISE;
    pipelineInfo.RasterizerStateInfo.DepthBiasEnable        = false;
    pipelineInfo.RasterizerStateInfo.LineWidth              = 1.0f;

    pipelineInfo.DepthStencilStateInfo = {};
    pipelineInfo.DepthStencilStateInfo.DepthTestEnabled     = false;
    pipelineInfo.DepthStencilStateInfo.DepthWriteEnabled    = true;
    pipelineInfo.DepthStencilStateInfo.StencilTestEnabled   = false;

    BlendRenderTargetInfo rtvBlendInfo = {};
    rtvBlendInfo.BlendEnabled           = false;
    rtvBlendInfo.SrcColorBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstColorBlendFactor    = BLEND_FACTOR::ONE_MINUS_SRC_ALPHA;
    rtvBlendInfo.ColorBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.SrcAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.AlphaBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.ColorWriteMask         = COLOR_WRITE_MASK::ENABLE_ALL;

    pipelineInfo.BlendStateInfo = {};
    pipelineInfo.BlendStateInfo.RenderTargetBlendInfos  = { rtvBlendInfo };
    pipelineInfo.BlendStateInfo.IndependentBlendEnabled = false;
    std::fill_n(pipelineInfo.BlendStateInfo.pBlendConstants, 4u, 1.0f);

    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void UIRenderer::OnPanelAdded(Entity entity)
{
    LOG_INFO("hello there");
    m_CommandListsToReset = MAX_FRAMES_IN_FLIGHT;

    UIPanelComponent& panel = ECSCore::GetInstance()->GetComponent<UIPanelComponent>(entity);

    PanelRenderResources panelRenderResources = {};

    // Create panel buffer
    const BufferInfo bufferInfo = {
        .ByteSize =
            sizeof(DirectX::XMFLOAT2) * 2 + // Position and size
            sizeof(DirectX::XMFLOAT4) +     // Highlight color
            sizeof(float),                  // Highlight factor
        .CPUAccess    = BUFFER_DATA_ACCESS::WRITE,
        .GPUAccess    = BUFFER_DATA_ACCESS::READ,
        .Usage        = BUFFER_USAGE::UNIFORM_BUFFER
    };

    panelRenderResources.pBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!panelRenderResources.pBuffer) {
        return;
    }

    // Create descriptor set
    panelRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayout);
    if (!panelRenderResources.pDescriptorSet) {
        return;
    }

    panelRenderResources.pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, panelRenderResources.pBuffer);
    panelRenderResources.pDescriptorSet->updateCombinedTextureSamplerDescriptor(SHADER_BINDING::TEXTURE_ONE, panel.texture, m_pAniSampler);

    m_PanelRenderResources.push_back(panelRenderResources, entity);
}

void UIRenderer::OnPanelRemoved(Entity entity)
{
    m_CommandListsToReset = MAX_FRAMES_IN_FLIGHT;

    PanelRenderResources panelRenderResources = m_PanelRenderResources.IndexID(entity);
    m_PanelRenderResources.Pop(entity);

    std::function<void()> deleteFunction = [this, panelRenderResources]() {
        m_pRenderingHandler->waitAllFrames();
        delete panelRenderResources.pBuffer;
        delete panelRenderResources.pDescriptorSet;
    };

    // TODO: Execute this on a separate thread. Currently, this needs to be done on the main thread to block
    // it from deleting a panel texture being used in rendering.
    deleteFunction();
}

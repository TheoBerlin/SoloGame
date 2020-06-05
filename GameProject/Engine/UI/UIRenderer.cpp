#include "UIRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>

UIRenderer::UIRenderer(ECSCore* pECS, Device* pDevice)
    :Renderer(pECS, pDevice),
    m_pCommandPool(nullptr),
    m_pCommandList(nullptr),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencil(pDevice->getDepthStencil()),
    m_pQuad(nullptr),
    m_pDescriptorSetLayoutCommon(nullptr),
    m_pDescriptorSetLayoutPanel(nullptr),
    m_pDescriptorSetCommon(nullptr),
    m_pAniSampler(nullptr),
    m_pRenderPass(nullptr),
    m_pFramebuffer(nullptr),
    m_pPipelineLayout(nullptr),
    m_pPipeline(nullptr)
{
    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, tid_UIPanel}}, &m_Panels, [this](Entity entity){ onPanelAdded(entity); }, [this](Entity entity){ onPanelRemoved(entity); }}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

UIRenderer::~UIRenderer()
{
    // Delete all panel render resources
    for (Entity entity : m_Panels.getIDs()) {
        onPanelRemoved(entity);
    }

    delete m_pCommandList;
    delete m_pCommandPool;
    delete m_pDescriptorSetCommon;
    delete m_pDescriptorSetLayoutCommon;
    delete m_pDescriptorSetLayoutPanel;
    delete m_pRenderPass;
    delete m_pFramebuffer;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool UIRenderer::init()
{
    m_pCommandPool = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
    if (!m_pCommandPool) {
        return false;
    }

    m_pCommandPool->allocateCommandLists(&m_pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY);
    if (!m_pCommandList) {
        return false;
    }

    m_pUIHandler = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    if (!m_pUIHandler) {
        return false;
    }

    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));
    m_pQuad         = pShaderResourceHandler->getQuarterScreenQuad();
    m_pAniSampler   = pShaderResourceHandler->getAniSampler();

    if (!createDescriptorSetLayouts()) {
        return false;
    }

    if (!createCommonDescriptorSet()) {
        return false;
    }

    if (!createRenderPass()) {
        return false;
    }

    if (!createFramebuffer()) {
        return false;
    }

    return createPipeline();
}

void UIRenderer::updateBuffers()
{
    size_t bufferSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float);              // Highlight factor

    for (const Entity& entity : m_Panels.getIDs()) {
        UIPanel& panel = m_pUIHandler->panels.indexID(entity);
        PanelRenderResources& panelRenderResources = m_PanelRenderResources.indexID(entity);

        // Set per-object buffer
        void* pMappedBuffer = nullptr;
        m_pCommandList->map(panelRenderResources.pBuffer, &pMappedBuffer);
        memcpy(pMappedBuffer, &panel, bufferSize);
        m_pCommandList->unmap(panelRenderResources.pBuffer);
    }
}

void UIRenderer::recordCommands()
{
    CommandListBeginInfo beginInfo = {};
    beginInfo.pRenderPass   = m_pRenderPass;
    beginInfo.Subpass       = 0u;
    beginInfo.pFramebuffer  = m_pFramebuffer;
    m_pCommandList->begin(COMMAND_LIST_USAGE::WITHIN_RENDER_PASS, &beginInfo);

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_pFramebuffer;
    m_pCommandList->beginRenderPass(m_pRenderPass, renderPassBeginInfo);

    if (m_Panels.size() == 0) {
        m_pCommandList->end();
        return;
    }

    m_pCommandList->bindPipeline(m_pPipeline);
    m_pCommandList->bindVertexBuffer(0, m_pQuad);

    m_pCommandList->bindDescriptorSet(m_pDescriptorSetCommon);

    for (const PanelRenderResources& panelRenderResources : m_PanelRenderResources.getVec()) {
        m_pCommandList->bindDescriptorSet(panelRenderResources.pDescriptorSet);
        m_pCommandList->draw(4);
    }

    m_pCommandList->end();
}

void UIRenderer::executeCommands()
{
    m_pDevice->graphicsQueueSubmit(m_pCommandList);
}

bool UIRenderer::createDescriptorSetLayouts()
{
    m_pDescriptorSetLayoutCommon = m_pDevice->createDescriptorSetLayout();
    if (!m_pDescriptorSetLayoutCommon) {
        return false;
    }

    m_pDescriptorSetLayoutCommon->addBindingSampler(SHADER_BINDING::SAMPLER_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    if (!m_pDescriptorSetLayoutCommon->finalize()) {
        return false;
    }

    m_pDescriptorSetLayoutPanel = m_pDevice->createDescriptorSetLayout();
    if (!m_pDescriptorSetLayoutPanel) {
        return false;
    }

    m_pDescriptorSetLayoutPanel->addBindingUniformBuffer(SHADER_BINDING::PER_OBJECT, SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayoutPanel->addBindingSampledTexture(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    return m_pDescriptorSetLayoutPanel->finalize();
}

bool UIRenderer::createCommonDescriptorSet()
{
    m_pDescriptorSetCommon = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutCommon);
    if (!m_pDescriptorSetCommon) {
        return false;
    }

    m_pDescriptorSetCommon->writeSamplerDescriptor(SHADER_BINDING::SAMPLER_ONE, m_pAniSampler);
    return true;
}

bool UIRenderer::createRenderPass()
{
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    Texture* pBackbuffer = m_pDevice->getBackBuffer();
    AttachmentInfo backBufferAttachment   = {};
    backBufferAttachment.Format           = pBackbuffer->getFormat();
    backBufferAttachment.Samples          = 1u;
    backBufferAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    backBufferAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    backBufferAttachment.InitialLayout    = TEXTURE_LAYOUT::RENDER_TARGET;
    backBufferAttachment.FinalLayout      = TEXTURE_LAYOUT::PRESENT;

    Texture* pDepthStencil = m_pDevice->getDepthStencil();
    AttachmentInfo depthStencilAttachment   = {};
    depthStencilAttachment.Format           = pDepthStencil->getFormat();
    depthStencilAttachment.Samples          = 1u;
    depthStencilAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    depthStencilAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    depthStencilAttachment.InitialLayout    = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;
    depthStencilAttachment.FinalLayout      = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;

    // Subpass
    SubpassInfo subpass = {};
    AttachmentReference backbufferRef   = {};
    backbufferRef.AttachmentIndex       = 0;
    backbufferRef.Layout                = TEXTURE_LAYOUT::RENDER_TARGET;

    AttachmentReference depthStencilRef = {};
    depthStencilRef.AttachmentIndex     = 1;
    depthStencilRef.Layout              = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;

    subpass.ColorAttachments        = { backbufferRef };
    subpass.DepthStencilAttachment  = { depthStencilRef };
    subpass.PipelineBindPoint       = PIPELINE_BIND_POINT::GRAPHICS;

    // Subpass dependency
    SubpassDependency subpassDependency = {};
    subpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    subpassDependency.DstSubpass        = 0;
    subpassDependency.SrcStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.DstStage          = PIPELINE_STAGE::FRAGMENT_SHADER;
    subpassDependency.SrcAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    renderPassInfo.AttachmentInfos  = { backBufferAttachment, depthStencilAttachment };
    renderPassInfo.Subpasses        = { subpass };
    renderPassInfo.Dependencies     = { subpassDependency };

    m_pRenderPass = m_pDevice->createRenderPass(renderPassInfo);
    return m_pRenderPass;
}

bool UIRenderer::createFramebuffer()
{
    Texture* pBackbuffer = m_pDevice->getBackBuffer();

    FramebufferInfo framebufferInfo = {};
    framebufferInfo.pRenderPass     = m_pRenderPass;
    framebufferInfo.Attachments     = { m_pDevice->getBackBuffer(), m_pDevice->getDepthStencil() };
    framebufferInfo.Dimensions      = pBackbuffer->getDimensions();

    m_pFramebuffer = m_pDevice->createFramebuffer(framebufferInfo);
    return m_pFramebuffer;
}

bool UIRenderer::createPipeline()
{
    m_pPipelineLayout = m_pDevice->createPipelineLayout({ m_pDescriptorSetLayoutCommon, m_pDescriptorSetLayoutPanel });
    if (!m_pPipelineLayout) {
        return false;
    }

    PipelineInfo pipelineInfo = {};
    pipelineInfo.ShaderInfos = {
        {"UI", SHADER_TYPE::VERTEX_SHADER},
        {"UI", SHADER_TYPE::FRAGMENT_SHADER}
    };

    pipelineInfo.PrimitiveTopology = PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP;

    const glm::uvec2& backbufferDims = m_pDevice->getBackBuffer()->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = (float)backbufferDims.x;
    viewport.Height   = (float)backbufferDims.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pipelineInfo.Viewports = { viewport };

    pipelineInfo.RasterizerStateInfo = {};
    pipelineInfo.RasterizerStateInfo.PolygonMode          = POLYGON_MODE::FILL;
    pipelineInfo.RasterizerStateInfo.CullMode             = CULL_MODE::NONE;
    pipelineInfo.RasterizerStateInfo.FrontFaceOrientation = FRONT_FACE_ORIENTATION::CLOCKWISE;
    pipelineInfo.RasterizerStateInfo.DepthBiasEnable      = false;

    pipelineInfo.DepthStencilStateInfo = {};
    pipelineInfo.DepthStencilStateInfo.DepthTestEnabled     = true;
    pipelineInfo.DepthStencilStateInfo.DepthWriteEnabled    = true;
    pipelineInfo.DepthStencilStateInfo.DepthComparisonFunc  = COMPARISON_FUNC::LESS;
    pipelineInfo.DepthStencilStateInfo.StencilTestEnabled   = false;


    BlendRenderTargetInfo rtvBlendInfo = {};
    rtvBlendInfo.BlendEnabled           = true;
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
    for (float& blendConstant : pipelineInfo.BlendStateInfo.pBlendConstants) {
        blendConstant = 1.0f;
    }

    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void UIRenderer::onPanelAdded(Entity entity)
{
    UIPanel& panel = m_pUIHandler->panels.indexID(entity);

    PanelRenderResources panelRenderResources = {};

    // Create panel buffer
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float) +             // Highlight factor
        sizeof(DirectX::XMFLOAT3    // Padding
    );
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage        = BUFFER_USAGE::UNIFORM_BUFFER;

    panelRenderResources.pBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!panelRenderResources.pBuffer) {
        return;
    }

    // Create descriptor set
    panelRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutPanel);
    if (!panelRenderResources.pDescriptorSet) {
        return;
    }

    panelRenderResources.pDescriptorSet->writeUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, panelRenderResources.pBuffer);
    panelRenderResources.pDescriptorSet->writeSampledTextureDescriptor(SHADER_BINDING::TEXTURE_ONE, panel.texture);

    m_PanelRenderResources.push_back(panelRenderResources, entity);
}

void UIRenderer::onPanelRemoved(Entity entity)
{
    PanelRenderResources& panelRenderResources = m_PanelRenderResources.indexID(entity);
    delete panelRenderResources.pBuffer;
    delete panelRenderResources.pDescriptorSet;

    m_PanelRenderResources.pop(entity);
}

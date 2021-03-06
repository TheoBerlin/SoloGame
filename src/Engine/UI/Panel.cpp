#include "Panel.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>

const RESOURCE_FORMAT g_PanelTextureFormat = RESOURCE_FORMAT::R8G8B8A8_UNORM;

UIHandler::UIHandler(ECSCore* pECS, Device* pDevice)
    :ComponentHandler(pECS, TID(UIHandler)),
    m_pDevice(pDevice),
    m_pCommandPool(nullptr),
    m_pCommandList(nullptr),
    m_pQuadVertices(nullptr),
    m_pAniSampler(nullptr),
    m_pDescriptorSetLayout(nullptr),
    m_pRenderPass(nullptr),
    m_pPipelineLayout(nullptr),
    m_pPipeline(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_UIPanel, &panels, [this](Entity entity){ delete panels.indexID(entity).texture; }},
        {tid_UIButton, &buttons}
    };

    handlerReg.HandlerDependencies = {
        TID(ShaderResourceHandler)
    };

    this->registerHandler(handlerReg);
}

UIHandler::~UIHandler()
{
    std::vector<UIPanel>& panelVec = panels.getVec();

    for (UIPanel& panel : panelVec) {
        delete panel.texture;
    }

    delete m_pCommandList;
    delete m_pCommandPool;
    delete m_pDescriptorSetLayout;
    delete m_pRenderPass;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool UIHandler::initHandler()
{
    m_pCommandPool = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
    if (!m_pCommandPool) {
        return false;
    }

    m_pCommandPool->allocateCommandLists(&m_pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY);
    if (!m_pCommandList) {
        return false;
    }

    // Retrieve quad from shader resource handler
    ShaderResourceHandler* pShaderResourceHandler = reinterpret_cast<ShaderResourceHandler*>(m_pECS->getEntityPublisher()->getComponentHandler(TID(ShaderResourceHandler)));
    // Retrieve UI rendering shader program from shader handler
    if (!pShaderResourceHandler) {
        return false;
    }

    m_pQuadVertices = pShaderResourceHandler->getQuarterScreenQuad();
    m_pAniSampler   = pShaderResourceHandler->getAniSampler();

    if (!createRenderPass()) {
        return false;
    }

    if (!createDescriptorSetLayout()) {
        return false;
    }

    return createPipeline();
}

void UIHandler::createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 highlight, float highlightFactor)
{
    UIPanel panel;
    panel.position = pos;
    panel.size = size;
    panel.highlightFactor = highlightFactor;
    panel.highlight = highlight;
    createPanelTexture(panel);

    panels.push_back(panel, entity);
    this->registerComponent(entity, tid_UIPanel);
}

void UIHandler::attachTextures(Entity entity, const TextureAttachmentInfo* pAttachmentInfos, std::shared_ptr<Texture>* pTextureReferences, size_t textureCount)
{
    if (!panels.hasElement(entity)) {
        LOG_WARNINGF("Tried to attach textures to a non-existing UI panel, entity: %d", entity);
        return;
    }

    UIPanel& panel = panels.indexID(entity);

    std::vector<TextureAttachment> attachments(textureCount);

    for (size_t textureIdx = 0; textureIdx < textureCount; textureIdx++) {
        createTextureAttachment(attachments[textureIdx], pAttachmentInfos[textureIdx], pTextureReferences[textureIdx], panel);
    }

    renderTexturesOntoPanel(attachments, panel);

    size_t oldTextureCount = panel.textures.size();
    panel.textures.resize(oldTextureCount + textureCount);

    for (TextureAttachment& txAttachment : attachments) {
        panel.textures[oldTextureCount++] = txAttachment;
    }
}

void UIHandler::createButton(Entity entity, DirectX::XMFLOAT4 hoverHighlight, DirectX::XMFLOAT4 pressHighlight, std::function<void()> onPress)
{
    if (!panels.hasElement(entity)) {
        LOG_WARNINGF("Tried to create a UI button for entity (%d) which does not have a UI panel", entity);
        return;
    }

    DirectX::XMFLOAT4 defaultHighlight = panels.indexID(entity).highlight;

    buttons.push_back({
        defaultHighlight,
        hoverHighlight,
        pressHighlight,
        onPress
    }, entity);

    this->registerComponent(entity, tid_UIButton);
}

bool UIHandler::createRenderPass()
{
    // Render pass attachments
    AttachmentInfo panelTextureAttachment   = {};
    panelTextureAttachment.Format           = g_PanelTextureFormat;
    panelTextureAttachment.Samples          = 1u;
    panelTextureAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    panelTextureAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    panelTextureAttachment.InitialLayout    = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    panelTextureAttachment.FinalLayout      = TEXTURE_LAYOUT::SHADER_READ_ONLY;

    // Subpass
    AttachmentReference panelTextureRef = {};
    panelTextureRef.AttachmentIndex     = 0u;
    panelTextureRef.Layout              = TEXTURE_LAYOUT::RENDER_TARGET;

    SubpassInfo subpass = {};
    subpass.ColorAttachments    = { panelTextureRef };
    subpass.PipelineBindPoint   = PIPELINE_BIND_POINT::GRAPHICS;

    // Subpass dependency
    SubpassDependency subpassDependency = {};
    subpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    subpassDependency.DstSubpass        = 0u;
    subpassDependency.SrcStage          = PIPELINE_STAGE::BOTTOM_OF_PIPE;
    subpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.SrcAccessMask     = (RESOURCE_ACCESS)(0);
    subpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    RenderPassInfo renderPassInfo   = {};
    renderPassInfo.AttachmentInfos  = { panelTextureAttachment };
    renderPassInfo.Subpasses        = { subpass };
    renderPassInfo.Dependencies     = { subpassDependency };

    m_pRenderPass = m_pDevice->createRenderPass(renderPassInfo);
    return m_pRenderPass;
}

bool UIHandler::createDescriptorSetLayout()
{
    // Create a single layout, even if some descriptors are per-panel and per-texture attachment
    m_pDescriptorSetLayout = m_pDevice->createDescriptorSetLayout();
    m_pDescriptorSetLayout->addBindingUniformBuffer(SHADER_BINDING::PER_OBJECT, SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayout->addBindingCombinedTextureSampler(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    return m_pDescriptorSetLayout->finalize(m_pDevice);
}

bool UIHandler::createPipeline()
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

    pipelineInfo.BlendStateInfo = {};

    BlendRenderTargetInfo rtvBlendInfo = {};
    rtvBlendInfo.BlendEnabled           = true;
    rtvBlendInfo.SrcColorBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstColorBlendFactor    = BLEND_FACTOR::ONE_MINUS_SRC_ALPHA;
    rtvBlendInfo.ColorBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.SrcAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.AlphaBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.ColorWriteMask         = COLOR_WRITE_MASK::ENABLE_ALL;

    pipelineInfo.BlendStateInfo.RenderTargetBlendInfos  = { rtvBlendInfo };
    pipelineInfo.BlendStateInfo.IndependentBlendEnabled = false;
    std::fill_n(pipelineInfo.BlendStateInfo.pBlendConstants, 4u, 1.0f);

    pipelineInfo.DynamicStates  = { PIPELINE_DYNAMIC_STATE::VIEWPORT, PIPELINE_DYNAMIC_STATE::SCISSOR };
    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void UIHandler::createPanelTexture(UIPanel& panel)
{
    const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

    // Create underlying texture
    TextureInfo textureInfo = {};
    textureInfo.Dimensions  = { uint32_t(panel.size.x * backbufferDims.x), uint32_t(panel.size.y * backbufferDims.y) };
    textureInfo.Usage       = TEXTURE_USAGE::SAMPLED | TEXTURE_USAGE::RENDER_TARGET;
    textureInfo.Layout      = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    textureInfo.Format      = g_PanelTextureFormat;

    panel.texture = m_pDevice->createTexture(textureInfo);
}

void UIHandler::createTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, std::shared_ptr<Texture>& texture, const UIPanel& panel)
{
    attachment.texture = texture;

    // Set size
    if (attachmentInfo.sizeSetting == TX_SIZE_STRETCH) {
        attachment.size = {1.0f, 1.0f};
        attachment.position = {0.0f, 0.0f};
        return;
    } else if (attachmentInfo.sizeSetting == TX_SIZE_CLIENT_RESOLUTION_DEPENDENT) {
        // Get the resolution of the texture
        const glm::uvec2& txDimensions = texture->getDimensions();

        const DirectX::XMFLOAT2& panelSize = panel.size;
        const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

        attachment.size = {(float)txDimensions.x / ((float)backbufferDims.x * panelSize.x), (float)txDimensions.y / ((float)backbufferDims.y * panelSize.y)};
    } else if (attachmentInfo.sizeSetting == TX_SIZE_EXPLICIT) {
        attachment.size = attachmentInfo.explicitSize;
    }

    // Set position
    switch (attachmentInfo.horizontalAlignment) {
        case TX_HORIZONTAL_ALIGNMENT_LEFT:
            attachment.position.x = 0.0f;
            break;
        case TX_HORIZONTAL_ALIGNMENT_CENTER:
            attachment.position.x = 0.5f - attachment.size.x * 0.5f;
            break;
        case TX_HORIZONTAL_ALIGNMENT_RIGHT:
            attachment.position.x = 1.0f - attachment.size.x;
            break;
        case TX_HORIZONTAL_ALIGNMENT_EXPLICIT:
            attachment.position.x = attachmentInfo.explicitPosition.x;
            break;
    }

    switch (attachmentInfo.verticalAlignment) {
        case TX_VERTICAL_ALIGNMENT_TOP:
            attachment.position.y = 1.0f - attachment.size.y;
            break;
        case TX_VERTICAL_ALIGNMENT_CENTER:
            attachment.position.y = 0.5f - attachment.size.y * 0.5f;
            break;
        case TX_VERTICAL_ALIGNMENT_BOTTOM:
            attachment.position.y = 0.0f;
            break;
        case TX_VERTICAL_ALIGNMENT_EXPLICIT:
            attachment.position.y = attachmentInfo.explicitPosition.y;
            break;
    }
}

void UIHandler::renderTexturesOntoPanel(std::vector<TextureAttachment>& attachments, UIPanel& panel)
{
    std::vector<AttachmentRenderResources> renderResources;
    if (!createPanelRenderResources(renderResources, attachments)) {
        LOG_ERROR("Failed to render textures onto panel, could not create create render resources");
        return;
    }

    Framebuffer* pFramebuffer = createFramebuffer(panel);

    CommandListBeginInfo beginInfo = {};
    beginInfo.pRenderPass   = m_pRenderPass;
    beginInfo.Subpass       = 0u;
    beginInfo.pFramebuffer  = pFramebuffer;
    m_pCommandList->begin(COMMAND_LIST_USAGE::ONE_TIME_SUBMIT | COMMAND_LIST_USAGE::WITHIN_RENDER_PASS, &beginInfo);

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = pFramebuffer;
    m_pCommandList->beginRenderPass(m_pRenderPass, renderPassBeginInfo);

    m_pCommandList->bindPipeline(m_pPipeline);

    const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX    = 0;
    viewport.TopLeftY    = 0;
    viewport.Width       = panel.size.x * backbufferDims.x;
    viewport.Height      = panel.size.y * backbufferDims.y;
    viewport.MinDepth    = 0.0f;
    viewport.MaxDepth    = 1.0f;
    m_pCommandList->bindViewport(&viewport);

    Rectangle2D scissorRectangle = {};
    scissorRectangle.Extent = { viewport.Width, viewport.Height };
    m_pCommandList->bindScissor(scissorRectangle);

    m_pCommandList->bindVertexBuffer(0, m_pQuadVertices);

    for (AttachmentRenderResources& attachmentResources : renderResources) {
        m_pCommandList->bindDescriptorSet(attachmentResources.pDescriptorSet, m_pPipelineLayout, 0u);
        m_pCommandList->draw(4u);
    }

    m_pCommandList->endRenderPass();
    m_pCommandList->end();

    IFence* pFence = m_pDevice->createFence(false);
    m_pDevice->graphicsQueueSubmit(m_pCommandList, pFence, nullptr);

    // Delete render resources when rendering has finished
    std::thread deleterThread = std::thread([renderResources, pFramebuffer, pFence, this]() mutable {
        m_pDevice->waitForFences(&pFence, 1u, false, UINT64_MAX);

        for (AttachmentRenderResources& attachmentResources : renderResources) {
            delete attachmentResources.pDescriptorSet;
            delete attachmentResources.pAttachmentBuffer;
        }

        delete pFramebuffer;
        delete pFence;
    });

    deleterThread.detach();
}

bool UIHandler::createPanelRenderResources(std::vector<AttachmentRenderResources>& renderResources, std::vector<TextureAttachment>& attachments)
{
    renderResources.reserve(attachments.size());

    struct BufferData {
        DirectX::XMFLOAT2 position, size;
        DirectX::XMFLOAT4 highlight;
        float highlightFactor;
    };

    BufferData bufferData = {};

    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize = sizeof(BufferData);
    bufferInfo.GPUAccess = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage = BUFFER_USAGE::UNIFORM_BUFFER;
    bufferInfo.pData = &bufferData;

    for (const TextureAttachment& attachment : attachments) {
        bufferData.position = attachment.position;
        bufferData.size     = attachment.size;

        IBuffer* pPerAttachmentBuffer = m_pDevice->createBuffer(bufferInfo);
        if (!pPerAttachmentBuffer) {
            LOG_ERROR("Failed to create per-object uniform buffer");
            return false;
        }

        DescriptorSet* pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayout);
        if (!pDescriptorSet) {
            return false;
        }

        pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, pPerAttachmentBuffer);
        pDescriptorSet->updateCombinedTextureSamplerDescriptor(SHADER_BINDING::TEXTURE_ONE, attachment.texture.get(), m_pAniSampler);

        renderResources.push_back({pDescriptorSet, pPerAttachmentBuffer});
    }

    return true;
}

Framebuffer* UIHandler::createFramebuffer(UIPanel& panel)
{
    const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

    FramebufferInfo framebufferInfo = {};
    framebufferInfo.pRenderPass     = m_pRenderPass;
    framebufferInfo.Dimensions      = { uint32_t(panel.size.x * (float)backbufferDims.x), uint32_t(panel.size.y * (float)backbufferDims.y) };
    framebufferInfo.Attachments     = { panel.texture };

    return m_pDevice->createFramebuffer(framebufferInfo);
}

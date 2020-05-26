#include "Panel.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/BlendState.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>
#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/APIAbstractions/Pipeline.hpp>
#include <Engine/Rendering/APIAbstractions/PipelineLayout.hpp>
#include <Engine/Rendering/APIAbstractions/RenderPass.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

const RESOURCE_FORMAT g_PanelTextureFormat = RESOURCE_FORMAT::R8G8B8A8_UNORM;

UIHandler::UIHandler(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :ComponentHandler(pECS, TID(UIHandler)),
    m_pDevice(pDevice),
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
    delete m_pDescriptorSetLayout;
    delete m_pRenderPass;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool UIHandler::initHandler()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
        return false;
    }

    // Retrieve quad from shader resource handler
    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderResourceHandler)));
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
        LOG_WARNING("Tried to attach textures to a non-existing UI panel, entity: %d", entity);
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
        LOG_WARNING("Tried to create a UI button for entity (%d) which does not have a UI panel", entity);
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
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    AttachmentInfo panelTextureAttachment   = {};
    panelTextureAttachment.Format           = g_PanelTextureFormat;
    panelTextureAttachment.Samples          = 1u;
    panelTextureAttachment.LoadOp           = ATTACHMENT_LOAD_OP::LOAD;
    panelTextureAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    panelTextureAttachment.InitialLayout    = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    panelTextureAttachment.FinalLayout      = TEXTURE_LAYOUT::SHADER_READ_ONLY;

    // Subpass
    SubpassInfo subpass = {};
    AttachmentReference panelTextureRef = {};
    panelTextureRef.AttachmentIndex     = 0u;
    panelTextureRef.Layout              = TEXTURE_LAYOUT::RENDER_TARGET;

    subpass.ColorAttachments    = { panelTextureRef };
    subpass.PipelineBindPoint   = PIPELINE_BIND_POINT::GRAPHICS;

    // Subpass dependency
    SubpassDependency subpassDependency = {};
    subpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    subpassDependency.DstSubpass        = 0;
    subpassDependency.SrcStage          = PIPELINE_STAGE::BOTTOM_OF_PIPE;
    subpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.SrcAccessMask     = RESOURCE_ACCESS::SHADER_READ;
    subpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

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
    m_pDescriptorSetLayout->addBindingSampler(SHADER_BINDING::SAMPLER_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayout->addBindingSampledTexture(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);
    return m_pDescriptorSetLayout->finalize();
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
    pipelineInfo.RasterizerStateInfo.PolygonMode          = POLYGON_MODE::FILL;
    pipelineInfo.RasterizerStateInfo.CullMode             = CULL_MODE::NONE;
    pipelineInfo.RasterizerStateInfo.FrontFaceOrientation = FRONT_FACE_ORIENTATION::CLOCKWISE;
    pipelineInfo.RasterizerStateInfo.DepthBiasEnable      = false;

    pipelineInfo.DepthStencilStateInfo = {};
    pipelineInfo.DepthStencilStateInfo.DepthTestEnabled     = true;
    pipelineInfo.DepthStencilStateInfo.DepthWriteEnabled    = true;
    pipelineInfo.DepthStencilStateInfo.DepthComparisonFunc  = COMPARISON_FUNC::LESS;
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

    pipelineInfo.BlendStateInfo.pRenderTargetBlendInfos = &rtvBlendInfo;
    pipelineInfo.BlendStateInfo.BlendInfosCount         = 1u;
    pipelineInfo.BlendStateInfo.IndependentBlendEnabled = false;
    for (float& blendConstant : pipelineInfo.BlendStateInfo.pBlendConstants) {
        blendConstant = 1.0f;
    }

    pipelineInfo.DynamicStates  = { PIPELINE_DYNAMIC_STATE::VIEWPORT };
    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void UIHandler::createPanelTexture(UIPanel& panel)
{
    const glm::uvec2& backbufferDims = m_pDevice->getBackBuffer()->getDimensions();

    // Create underlying texture
    TextureInfo textureInfo = {};
    textureInfo.Dimensions      = { uint32_t(panel.size.x * backbufferDims.x), uint32_t(panel.size.y * backbufferDims.y) };
    textureInfo.Format          = g_PanelTextureFormat;
    textureInfo.InitialLayout   = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    textureInfo.LayoutFlags     = TEXTURE_LAYOUT::SHADER_READ_ONLY | TEXTURE_LAYOUT::RENDER_TARGET;

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
        const glm::uvec2& backbufferDims = m_pDevice->getBackBuffer()->getDimensions();

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
    if (!createPanelRenderResources(renderResources, attachments, panel)) {
        LOG_ERROR("Failed to render textures onto panel, could not create create render resources");
        return;
    }

    IFramebuffer* pFramebuffer = createFramebuffer(panel);

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = pFramebuffer;
    m_pCommandList->beginRenderPass(m_pRenderPass, renderPassBeginInfo);

    m_pCommandList->bindPipeline(m_pPipeline);

    const glm::uvec2& backbufferDims = m_pDevice->getBackBuffer()->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX    = 0;
    viewport.TopLeftY    = 0;
    viewport.Width       = panel.size.x * backbufferDims.x;
    viewport.Height      = panel.size.y * backbufferDims.y;
    viewport.MinDepth    = 0.0f;
    viewport.MaxDepth    = 1.0f;
    m_pCommandList->bindViewport(&viewport);

    m_pCommandList->bindVertexBuffer(0, m_pQuadVertices);

    for (AttachmentRenderResources& attachmentResources : renderResources) {
        m_pCommandList->bindDescriptorSet(attachmentResources.pDescriptorSet);
        m_pCommandList->draw(4u);
    }

    m_pCommandList->execute();

    // Delete render resources
    for (AttachmentRenderResources& attachmentResources : renderResources) {
        delete attachmentResources.pDescriptorSet;
        delete attachmentResources.pAttachmentBuffer;
    }

    delete pFramebuffer;
}

bool UIHandler::createPanelRenderResources(std::vector<AttachmentRenderResources>& renderResources, std::vector<TextureAttachment>& attachments, UIPanel& panel)
{
    renderResources.reserve(attachments.size());

    struct BufferData {
        DirectX::XMFLOAT2 position, size;
        DirectX::XMFLOAT4 highlight;
        float highlightFactor;
    };

    BufferData bufferData = {};

    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize =
        sizeof(DirectX::XMFLOAT2) * 2 + // Position and size
        sizeof(DirectX::XMFLOAT4) +     // Highlight color
        sizeof(float) +                 // Highlight factor
        sizeof(DirectX::XMFLOAT3);      // Padding
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

        pDescriptorSet->writeUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, pPerAttachmentBuffer);
        pDescriptorSet->writeSamplerDescriptor(SHADER_BINDING::SAMPLER_ONE, m_pAniSampler);
        pDescriptorSet->writeSampledTextureDescriptor(SHADER_BINDING::TEXTURE_ONE, attachment.texture.get());

        renderResources.push_back({pDescriptorSet, pPerAttachmentBuffer});
    }

    return true;
}

IFramebuffer* UIHandler::createFramebuffer(UIPanel& panel)
{
    FramebufferInfo framebufferInfo = {};
    framebufferInfo.pRenderPass     = m_pRenderPass;
    framebufferInfo.Dimensions      = {(uint32_t)panel.size.x, (uint32_t)panel.size.y};
    framebufferInfo.Attachments     = { panel.texture };

    return m_pDevice->createFramebuffer(framebufferInfo);
}

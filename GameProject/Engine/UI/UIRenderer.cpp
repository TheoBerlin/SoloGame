#include "UIRenderer.hpp"

#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>
#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/APIAbstractions/RenderPass.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIRenderer::UIRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :Renderer(pECS, pDevice),
    m_pCommandList(nullptr),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencil(pDevice->getDepthStencil()),
    m_BackbufferWidth(pWindow->getWidth()),
    m_BackbufferHeight(pWindow->getHeight()),
    m_pQuad(nullptr),
    m_pDescriptorSetLayoutCommon(nullptr),
    m_pDescriptorSetLayoutPanel(nullptr),
    m_pDescriptorSetCommon(nullptr),
    m_pAniSampler(nullptr),
    m_pRasterizerState(nullptr),
    m_pRenderPass(nullptr),
    m_pFramebuffer(nullptr)
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
    delete m_pDescriptorSetCommon;
    delete m_pDescriptorSetLayoutCommon;
    delete m_pDescriptorSetLayoutPanel;
    delete m_pRasterizerState;
    delete m_pRenderPass;
    delete m_pFramebuffer;
}

bool UIRenderer::init()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
        return false;
    }

    m_pShaderHandler    = static_cast<ShaderHandler*>(getComponentHandler(TID(ShaderHandler)));
    m_pUIHandler        = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    if (!m_pShaderHandler || !m_pUIHandler) {
        return false;
    }

    m_pUIProgram = m_pShaderHandler->getProgram(UI);

    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));
    m_pQuad = pShaderResourceHandler->getQuarterScreenQuad();
    m_pAniSampler = pShaderResourceHandler->getAniSampler();

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

    RasterizerStateInfo rsInfo = {};
    rsInfo.PolygonMode          = POLYGON_MODE::FILL;
    rsInfo.CullMode             = CULL_MODE::NONE;
    rsInfo.FrontFaceOrientation = FRONT_FACE_ORIENTATION::CLOCKWISE;
    rsInfo.DepthBiasEnable      = false;

    m_pRasterizerState = m_pDevice->createRasterizerState(rsInfo);
    if (!m_pRasterizerState) {
        LOG_ERROR("Failed to create rasterizer state");
        return false;
    }

    // Create viewport
    m_Viewport = {};
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width    = (float)m_BackbufferWidth;
    m_Viewport.Height   = (float)m_BackbufferHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    return true;
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
    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_pFramebuffer;
    m_pCommandList->beginRenderPass(m_pRenderPass, renderPassBeginInfo);

    if (m_Panels.size() == 0) {
        return;
    }

    m_pCommandList->bindPrimitiveTopology(PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP);
    m_pCommandList->bindInputLayout(m_pUIProgram->pInputLayout);
    m_pCommandList->bindVertexBuffer(0, m_pUIProgram->pInputLayout->getVertexSize(), m_pQuad);

    m_pCommandList->bindShaders(m_pUIProgram);

    m_pCommandList->bindRasterizerState(m_pRasterizerState);
    m_pCommandList->bindViewport(&m_Viewport);

    m_pCommandList->bindDescriptorSet(m_pDescriptorSetCommon);

    for (const PanelRenderResources& panelRenderResources : m_PanelRenderResources.getVec()) {
        m_pCommandList->bindDescriptorSet(panelRenderResources.pDescriptorSet);
        m_pCommandList->draw(4);
    }
}

void UIRenderer::executeCommands()
{
    m_pCommandList->execute();
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

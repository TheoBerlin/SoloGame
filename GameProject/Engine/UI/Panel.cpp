#include "Panel.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/BlendState.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIHandler::UIHandler(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :ComponentHandler(pECS, TID(UIHandler)),
    m_ClientWidth(pWindow->getWidth()),
    m_ClientHeight(pWindow->getHeight()),
    m_pDevice(pDevice),
    m_pCommandList(nullptr),
    m_pQuadVertices(nullptr),
    m_pAniSampler(nullptr),
    m_pBlendState(nullptr),
    m_pRasterizerState(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_UIPanel, &panels, [this](Entity entity){ delete panels.indexID(entity).texture; }},
        {tid_UIButton, &buttons}
    };

    handlerReg.HandlerDependencies = {
        TID(ShaderResourceHandler),
        TID(ShaderHandler)
    };

    this->registerHandler(handlerReg);
}

UIHandler::~UIHandler()
{
    std::vector<UIPanel>& panelVec = panels.getVec();

    for (UIPanel& panel : panelVec) {
        delete panel.texture;
    }

    delete m_pPerObjectBuffer;
    delete m_pCommandList;
    delete m_pRasterizerState;
    delete m_pBlendState;
}

bool UIHandler::initHandler()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
        return false;
    }

    // Retrieve quad from shader resource handler
    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderResourceHandler)));
    m_pQuadVertices = pShaderResourceHandler->getQuarterScreenQuad();

    // Retrieve UI rendering shader program from shader handler
    ShaderHandler* pShaderHandler = static_cast<ShaderHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderHandler)));
    if (!pShaderResourceHandler || !pShaderHandler) {
        return false;
    }

    m_pUIProgram = pShaderHandler->getProgram(PROGRAM::UI);
    m_pAniSampler = pShaderResourceHandler->getAniSampler();

    // Create constant buffer for texture rendering
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize =
        sizeof(DirectX::XMFLOAT2) * 2 + // Position and size
        sizeof(DirectX::XMFLOAT4) +     // Highlight color
        sizeof(float) +                 // Highlight factor
        sizeof(DirectX::XMFLOAT3);      // Padding
    bufferInfo.GPUAccess = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage = BUFFER_USAGE::UNIFORM_BUFFER;

    m_pPerObjectBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!m_pPerObjectBuffer) {
        LOG_ERROR("Failed to create per-object uniform buffer");
        return false;
    }

    BlendRenderTargetInfo rtvBlendInfo = {};
    rtvBlendInfo.BlendEnabled           = true;
    rtvBlendInfo.SrcColorBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstColorBlendFactor    = BLEND_FACTOR::ONE_MINUS_SRC_ALPHA;
    rtvBlendInfo.ColorBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.SrcAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.AlphaBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.ColorWriteMask         = COLOR_WRITE_MASK::ENABLE_ALL;

    BlendStateInfo blendStateInfo = {};
    blendStateInfo.pRenderTargetBlendInfos = &rtvBlendInfo;
    blendStateInfo.BlendInfosCount = 1u;
    blendStateInfo.IndependentBlendEnabled = false;
    for (float& blendConstant : blendStateInfo.pBlendConstants) {
        blendConstant = 1.0f;
    }

    m_pBlendState = m_pDevice->createBlendState(blendStateInfo);
    if (!m_pBlendState) {
        LOG_ERROR("Failed to create blend state");
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

    return true;
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

void UIHandler::createPanelTexture(UIPanel& panel)
{
    // Create underlying texture
    TextureInfo textureInfo = {};
    textureInfo.Dimensions      = {uint32_t(panel.size.x * m_ClientWidth), uint32_t(panel.size.y * m_ClientHeight)};
    textureInfo.Format          = RESOURCE_FORMAT::R8G8B8A8_UNORM;
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
        attachment.size = {(float)txDimensions.x / ((float)m_ClientWidth * panelSize.x), (float)txDimensions.y / ((float)m_ClientHeight * panelSize.y)};
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
    Viewport viewport = {};
    viewport.TopLeftX    = 0;
    viewport.TopLeftY    = 0;
    viewport.Width       = panel.size.x * m_ClientWidth;
    viewport.Height      = panel.size.y * m_ClientHeight;
    viewport.MinDepth    = 0.0f;
    viewport.MaxDepth    = 1.0f;
    m_pCommandList->bindViewport(&viewport);

    // Rendering setup
    m_pCommandList->bindShaders(m_pUIProgram);

    m_pCommandList->bindPrimitiveTopology(PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP);
    m_pCommandList->bindInputLayout(m_pUIProgram->pInputLayout);

    m_pCommandList->bindVertexBuffer(0, m_pUIProgram->pInputLayout->getVertexSize(), m_pQuadVertices);
    m_pCommandList->bindBuffer(0, SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER, m_pPerObjectBuffer);
    m_pCommandList->bindSampler(0u, SHADER_TYPE::FRAGMENT_SHADER, m_pAniSampler);

    m_pCommandList->bindRasterizerState(m_pRasterizerState);

    m_pCommandList->bindBlendState(m_pBlendState);

    // Set constant buffer data
    struct BufferData {
        DirectX::XMFLOAT2 position, size;
        DirectX::XMFLOAT4 highlight;
        float highlightFactor;
    };

    m_pCommandList->bindRenderTarget(panel.texture, nullptr);

    for (TextureAttachment& attachment : attachments) {
        m_pCommandList->bindShaderResourceTexture(0, SHADER_TYPE::FRAGMENT_SHADER, attachment.texture.get());

        BufferData bufferData = {
            attachment.position, attachment.size,
            {0.0f, 0.0f, 0.0f, 0.0f},               // No highlight color is desired
            0.0f
        };

        void* pMappedMemory = nullptr;
        m_pCommandList->map(m_pPerObjectBuffer, &pMappedMemory);
        std::memcpy(pMappedMemory, &bufferData, sizeof(BufferData));
        m_pCommandList->unmap(m_pPerObjectBuffer);

        m_pCommandList->draw(4u);
    }

    m_pCommandList->execute();
}

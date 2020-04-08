#include "Panel.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIHandler::UIHandler(ECSCore* pECS, Display* pDisplay)
    :ComponentHandler({tid_UIPanel}, pECS, std::type_index(typeid(UIHandler))),
    m_ClientWidth(pDisplay->getClientWidth()),
    m_ClientHeight(pDisplay->getClientHeight()),
    m_pDevice(pDisplay->getDevice()),
    m_pContext(pDisplay->getDeviceContext())
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

    m_pPerObjectBuffer->Release();
}

bool UIHandler::init()
{
    // Retrieve quad from shader resource handler
    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderResourceHandler)));
    m_Quad = pShaderResourceHandler->getQuarterScreenQuad();

    // Retrieve UI rendering shader program from shader handler
    ShaderHandler* pShaderHandler = static_cast<ShaderHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderHandler)));
    if (!pShaderResourceHandler || !pShaderHandler) {
        return false;
    }

    m_pUIProgram = pShaderHandler->getProgram(PROGRAM::UI);
    m_pAniSampler = pShaderResourceHandler->getAniSampler();

    // Create constant buffer for texture rendering
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth =
        sizeof(DirectX::XMFLOAT2) * 2 + // Position and size
        sizeof(DirectX::XMFLOAT4) +     // Highlight color
        sizeof(float) +                 // Highlight factor
        sizeof(DirectX::XMFLOAT3);      // Padding
    bufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags            = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags            = 0;
    bufferDesc.StructureByteStride  = 0;

    HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pPerObjectBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create per-char cbuffer: %s", hresultToString(hr).c_str());
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

void UIHandler::attachTextures(Entity entity, const TextureAttachmentInfo* pAttachmentInfos, TextureReference* pTextureReferences, size_t textureCount)
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
    std::memcpy(&panel.textures[oldTextureCount], attachments.data(), sizeof(TextureAttachment) * textureCount);
}

void UIHandler::createButton(Entity entity, DirectX::XMFLOAT4 hoverHighlight, DirectX::XMFLOAT4 pressHighlight,
    std::function<void()> onPress)
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
    D3D11_TEXTURE2D_DESC txDesc = {};
    txDesc.Width = UINT(panel.size.x * m_ClientWidth);
    txDesc.Height = UINT(panel.size.y * m_ClientHeight);
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Usage = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;

    ID3D11Texture2D* texture2D = nullptr;
    HRESULT hr = m_pDevice->CreateTexture2D(&txDesc, nullptr, &texture2D);
    if (hr != S_OK) {
        LOG_WARNING("Failed to create texture for UI panel: %s", hresultToString(hr).c_str());
        return;
    }

    // Create shader resource view
    ID3D11ShaderResourceView* pSRV = nullptr;
    hr = m_pDevice->CreateShaderResourceView(texture2D, nullptr, &pSRV);
    if (hr != S_OK) {
        LOG_WARNING("Failed to create shader resource view for UI panel: %s", hresultToString(hr).c_str());
    }

    texture2D->Release();
    panel.texture = new Texture(pSRV);
}

void UIHandler::createTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, const TextureReference& texture, const UIPanel& panel)
{
    attachment.texture = texture;

    // Set size
    if (attachmentInfo.sizeSetting == TX_SIZE_STRETCH) {
        attachment.size = {1.0f, 1.0f};
        attachment.position = {0.0f, 0.0f};
        return;
    } else if (attachmentInfo.sizeSetting == TX_SIZE_CLIENT_RESOLUTION_DEPENDENT) {
        // Get the resolution of the texture
        ID3D11ShaderResourceView* pSRV = texture.getSRV();
        ID3D11Resource* resource;
        pSRV->GetResource(&resource);

        ID3D11Texture2D* tx2D = static_cast<ID3D11Texture2D*>(resource);
        D3D11_TEXTURE2D_DESC txDesc = {};
        tx2D->GetDesc(&txDesc);
        resource->Release();

        const DirectX::XMFLOAT2& panelSize = panel.size;
        attachment.size = {(float)txDesc.Width / ((float)m_ClientWidth * panelSize.x), (float)txDesc.Height / ((float)m_ClientHeight * panelSize.y)};
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

void UIHandler::renderTexturesOntoPanel(const std::vector<TextureAttachment>& attachments, UIPanel& panel)
{
    // Get old viewport, and set a new one
    UINT viewportCount = 1;
    D3D11_VIEWPORT oldViewport = {};
    m_pContext->RSGetViewports(&viewportCount, &oldViewport);

    D3D11_VIEWPORT newViewport = {};
    newViewport.TopLeftX = 0;
    newViewport.TopLeftY = 0;
    newViewport.Width = panel.size.x * m_ClientWidth;
    newViewport.Height = panel.size.y * m_ClientHeight;
    newViewport.MinDepth = 0.0f;
    newViewport.MaxDepth = 1.0f;
    m_pContext->RSSetViewports(1, &newViewport);

    // Rendering setup
    m_pContext->VSSetShader(m_pUIProgram->vertexShader, nullptr, 0);
    m_pContext->HSSetShader(m_pUIProgram->hullShader, nullptr, 0);
    m_pContext->DSSetShader(m_pUIProgram->domainShader, nullptr, 0);
    m_pContext->GSSetShader(m_pUIProgram->geometryShader, nullptr, 0);
    m_pContext->PSSetShader(m_pUIProgram->pixelShader, nullptr, 0);

    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pContext->IASetInputLayout(m_pUIProgram->inputLayout);

    UINT offsets = 0;
    m_pContext->IASetVertexBuffers(0, 1, m_Quad.GetAddressOf(), &m_pUIProgram->vertexSize, &offsets);
    m_pContext->VSSetConstantBuffers(0, 1, &m_pPerObjectBuffer);
    m_pContext->PSSetConstantBuffers(0, 1, &m_pPerObjectBuffer);
    m_pContext->PSSetSamplers(0, 1, m_pAniSampler);
    m_pContext->OMSetDepthStencilState(nullptr, 0);

    // Set constant buffer data
    struct BufferData {
        DirectX::XMFLOAT2 position, size;
        DirectX::XMFLOAT4 highlight;
        float highlightFactor;
    };

    // Create a render target view from the panel texture
    ID3D11Resource* panelResource = nullptr;
    panel.texture->getSRV()->GetResource(&panelResource);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    ID3D11RenderTargetView* panelRtv = nullptr;
    HRESULT hr = m_pDevice->CreateRenderTargetView(panelResource, &rtvDesc, &panelRtv);
    if (hr != S_OK) {
        LOG_WARNING("Failed to create render target view of panel texture: %s", hresultToString(hr).c_str());
        return;
    }

    m_pContext->OMSetRenderTargets(1, &panelRtv, nullptr);

    for (const TextureAttachment& attachment : attachments) {
        ID3D11ShaderResourceView* pSRV = attachment.texture.getSRV();
        m_pContext->PSSetShaderResources(0, 1, &pSRV);

        BufferData bufferData = {
            attachment.position, attachment.size,
            {0.0f, 0.0f, 0.0f, 0.0f},               // No highlight color is desired
            0.0f
        };

        D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
        hr = m_pContext->Map(m_pPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
        if (hr != S_OK) {
            LOG_WARNING("Failed to map per-object constant buffer: %s", hresultToString(hr).c_str());
            panelRtv->Release();
            return;
        }

        std::memcpy(mappedBuffer.pData, &bufferData, sizeof(BufferData));
        m_pContext->Unmap(m_pPerObjectBuffer, 0);

        m_pContext->Draw(4, 0);
    }

    panelResource->Release();
    panelRtv->Release();

    // Reset viewport
    m_pContext->RSSetViewports(1, &oldViewport);
}

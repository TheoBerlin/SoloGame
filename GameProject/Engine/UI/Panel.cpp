#include "Panel.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Engine/Utils/DirectXUtils.hpp>

UIHandler::UIHandler(SystemSubscriber* pSysSubscriber, Display* pDisplay)
    :ComponentHandler({tid_UIPanel}, pSysSubscriber, std::type_index(typeid(UIHandler))),
    m_ClientWidth(pDisplay->getWindowWidth()),
    m_ClientHeight(pDisplay->getWindowHeight()),
    m_pDevice(pDisplay->getDevice()),
    m_pContext(pDisplay->getDeviceContext())
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_UIPanel, &panels},
        {tid_UIButton, &buttons}
    };

    this->registerHandler(&compRegs);

    std::type_index tid_textureLoader = std::type_index(typeid(TextureLoader));
    textureLoader = static_cast<TextureLoader*>(pSysSubscriber->getComponentHandler(tid_textureLoader));

    // Retrieve quad from shader resource handler
    std::type_index tid_shaderResourceHandler = std::type_index(typeid(ShaderResourceHandler));
    ShaderResourceHandler* shaderResourceHandler = static_cast<ShaderResourceHandler*>(pSysSubscriber->getComponentHandler(tid_shaderResourceHandler));

    m_Quad = shaderResourceHandler->getQuarterScreenQuad();

    // Retrieve UI rendering shader program from shader handler
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    ShaderHandler* shaderHandler = static_cast<ShaderHandler*>(pSysSubscriber->getComponentHandler(tid_shaderHandler));

    m_pUIProgram = shaderHandler->getProgram(PROGRAM::UI);
    m_pAniSampler = shaderResourceHandler->getAniSampler();

    // Create constant buffer for texture rendering
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float) +             // Highlight factor
        sizeof(DirectX::XMFLOAT3);  // Padding
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pPerObjectBuffer);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per-char cbuffer: %s", hresultToString(hr).c_str());
}

UIHandler::~UIHandler()
{
    std::vector<UIPanel>& panelVec = panels.getVec();

    for (UIPanel& panel : panelVec) {
        panel.texture->Release();
    }

    m_pPerObjectBuffer->Release();
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
    this->registerComponent(tid_UIPanel, entity);
}

void UIHandler::attachTexture(Entity entity, const TextureAttachmentInfo& attachmentInfo, ID3D11ShaderResourceView* texture)
{
    if (!panels.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to attach a texture to a non-existing UI panel, entity: %d", entity);
        return;
    }

    UIPanel& panel = panels.indexID(entity);

    TextureAttachment attachment = {};
    createTextureAttachment(attachment, attachmentInfo, texture, panel);
    renderTextureOntoPanel(attachment, panel);

    panel.textures.push_back(attachment);
}

void UIHandler::attachTexture(Entity entity, const TextureAttachmentInfo& attachmentInfo, std::string texturePath)
{
    ID3D11ShaderResourceView* texture = textureLoader->loadTexture(texturePath).srv;
    attachTexture(entity, attachmentInfo, texture);
}

void UIHandler::createButton(Entity entity, DirectX::XMFLOAT4 hoverHighlight, DirectX::XMFLOAT4 pressHighlight,
    std::function<void()> onPress)
{
    if (!panels.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to create a UI button for entity (%d) which does not have a UI panel", entity);
        return;
    }

    DirectX::XMFLOAT4 defaultHighlight = panels.indexID(entity).highlight;

    buttons.push_back({
        defaultHighlight,
        hoverHighlight,
        pressHighlight,
        onPress
    }, entity);

    this->registerComponent(tid_UIButton, entity);
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
        Logger::LOG_WARNING("Failed to create texture for UI panel: %s", hresultToString(hr).c_str());
        return;
    }

    // Create shader resource view
    hr = m_pDevice->CreateShaderResourceView(texture2D, nullptr, &panel.texture);
    if (hr != S_OK) {
        Logger::LOG_WARNING("Failed to create shader resource view for UI panel: %s", hresultToString(hr).c_str());
    }

    texture2D->Release();
}

void UIHandler::createTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, ID3D11ShaderResourceView* texture, const UIPanel& panel)
{
    attachment.texture = texture;

    // Set size
    if (attachmentInfo.sizeSetting == TX_SIZE_STRETCH) {
        attachment.size = {1.0f, 1.0f};
        attachment.position = {0.0f, 0.0f};
        return;
    } else if (attachmentInfo.sizeSetting == TX_SIZE_CLIENT_RESOLUTION_DEPENDENT) {
        // Get the resolution of the texture
        ID3D11Resource* resource;
        texture->GetResource(&resource);

        ID3D11Texture2D* tx2D = static_cast<ID3D11Texture2D*>(resource);
        D3D11_TEXTURE2D_DESC txDesc = {};
        tx2D->GetDesc(&txDesc);

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

void UIHandler::renderTextureOntoPanel(const TextureAttachment& attachment, UIPanel& panel)
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

    // Create a render target view from the panel texture
    ID3D11Resource* panelResource = nullptr;
    panel.texture->GetResource(&panelResource);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    ID3D11RenderTargetView* panelRtv = nullptr;
    HRESULT hr = m_pDevice->CreateRenderTargetView(panelResource, &rtvDesc, &panelRtv);
    if (hr != S_OK) {
        Logger::LOG_WARNING("Failed to create render target view of panel texture: %s", hresultToString(hr).c_str());
        return;
    }

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
    m_pContext->PSSetShaderResources(0, 1, &attachment.texture);
    m_pContext->PSSetSamplers(0, 1, m_pAniSampler);
    m_pContext->OMSetDepthStencilState(nullptr, 0);
    m_pContext->OMSetRenderTargets(1, &panelRtv, nullptr);

    // Set constant buffer data
    struct BufferData {
        DirectX::XMFLOAT2 position, size;
        DirectX::XMFLOAT4 highlight;
        float highlightFactor;
    };

    BufferData bufferData = {
        attachment.position, attachment.size,
        {0.0f, 0.0f, 0.0f, 0.0f},               // No highlight color is desired
        0.0f
    };

    D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
    hr = m_pContext->Map(m_pPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    if (hr != S_OK) {
        Logger::LOG_WARNING("Failed to map per-object constant buffer: %s", hresultToString(hr).c_str());
        panelRtv->Release();
        return;
    }

    std::memcpy(mappedBuffer.pData, &bufferData, sizeof(BufferData));
    m_pContext->Unmap(m_pPerObjectBuffer, 0);

    m_pContext->Draw(4, 0);

    panelResource->Release();
    panelRtv->Release();

    m_pContext->RSSetViewports(1, &oldViewport);
}

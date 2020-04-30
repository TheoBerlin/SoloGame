#include "UIRenderer.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIRenderer::UIRenderer(ECSCore* pECS, DeviceDX11* pDevice, Window* pWindow)
    :Renderer(pECS, pDevice),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencilView(pDevice->getDepthStencilView()),
    m_BackbufferWidth(pWindow->getWidth()),
    m_BackbufferHeight(pWindow->getHeight())
{
    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, tid_UIPanel}}, &m_Panels}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

UIRenderer::~UIRenderer()
{
    SAFERELEASE(m_pCommandBuffer);
}

bool UIRenderer::init()
{
    if (!createCommandBuffer(&m_pCommandBuffer)) {
        return false;
    }

    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));
    m_pShaderHandler = static_cast<ShaderHandler*>(getComponentHandler(TID(ShaderHandler)));
    m_pUIHandler = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    if (!m_pShaderHandler || !m_pUIHandler) {
        return false;
    }

    m_pUIProgram = m_pShaderHandler->getProgram(UI);

    m_Quad = pShaderResourceHandler->getQuarterScreenQuad();
    m_ppAniSampler = pShaderResourceHandler->getAniSampler();

    // Create per-panel constant buffer
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

    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(m_pDevice);
    ID3D11Device* pDevice = pDeviceDX->getDevice();

    HRESULT hr = pDevice->CreateBuffer(&bufferDesc, nullptr, m_PerPanelBuffer.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create per-object cbuffer: %s", hresultToString(hr).c_str());
        return false;
    }

    // Create viewport
    m_Viewport = {};
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = (float)m_BackbufferWidth;
    m_Viewport.Height = (float)m_BackbufferHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    return true;
}

void UIRenderer::recordCommands()
{
    if (m_Panels.size() == 0) {
        return;
    }

    m_pCommandBuffer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pCommandBuffer->IASetInputLayout(m_pUIProgram->inputLayout);
    UINT offsets = 0;
    m_pCommandBuffer->IASetVertexBuffers(0, 1, m_Quad.GetAddressOf(), &m_pUIProgram->vertexSize, &offsets);

    m_pCommandBuffer->VSSetShader(m_pUIProgram->vertexShader, nullptr, 0);
    m_pCommandBuffer->HSSetShader(m_pUIProgram->hullShader, nullptr, 0);
    m_pCommandBuffer->DSSetShader(m_pUIProgram->domainShader, nullptr, 0);
    m_pCommandBuffer->GSSetShader(m_pUIProgram->geometryShader, nullptr, 0);
    m_pCommandBuffer->PSSetShader(m_pUIProgram->pixelShader, nullptr, 0);

    m_pCommandBuffer->RSSetViewports(1, &m_Viewport);

    m_pCommandBuffer->PSSetSamplers(0, 1, m_ppAniSampler);
    m_pCommandBuffer->OMSetRenderTargets(1, &m_pRenderTarget, m_pDepthStencilView);

    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));

    size_t bufferSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float);              // Highlight factor

    for (const Entity& entity : m_Panels.getIDs()) {
        UIPanel& panel = m_pUIHandler->panels.indexID(entity);
        if (panel.texture->getSRV() == nullptr) {
            continue;
        }

        // Set per-object buffer
        m_pCommandBuffer->Map(m_PerPanelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
        memcpy(mappedResources.pData, &panel, bufferSize);
        m_pCommandBuffer->Unmap(m_PerPanelBuffer.Get(), 0);

        m_pCommandBuffer->VSSetConstantBuffers(0, 1, m_PerPanelBuffer.GetAddressOf());
        m_pCommandBuffer->PSSetConstantBuffers(0, 1, m_PerPanelBuffer.GetAddressOf());

        ID3D11ShaderResourceView* pPanelSRV = panel.texture->getSRV();
        m_pCommandBuffer->PSSetShaderResources(0, 1, &pPanelSRV);

        m_pCommandBuffer->Draw(4, 0);
    }
}

bool UIRenderer::executeCommands()
{
    return executeCommandBuffer(m_pCommandBuffer);
}

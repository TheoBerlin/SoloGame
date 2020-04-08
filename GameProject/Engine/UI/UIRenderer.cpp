#include "UIRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIRenderer::UIRenderer(ECSCore* pECS, Display* pDisplay)
    :System(pECS),
    m_pDevice(pDisplay->getDevice()),
    m_pContext(pDisplay->getDeviceContext()),
    m_pRenderTarget(pDisplay->getRenderTarget()),
    m_pDepthStencilView(pDisplay->getDepthStencilView()),
    m_BackbufferWidth(pDisplay->getClientWidth()),
    m_BackbufferHeight(pDisplay->getClientHeight())
{
    SystemRegistration sysReg = {
    {
        {{{R, tid_UIPanel}}, &m_Panels},
    },
    this};

    subscribeToComponents(sysReg);
}

UIRenderer::~UIRenderer()
{}

bool UIRenderer::init()
{
    const std::type_index tid_shaderResourceHandler = TID(ShaderResourceHandler);
    const std::type_index tid_shaderHandler = TID(ShaderHandler);
    const std::type_index tid_UIHandler = TID(UIHandler);

    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(tid_shaderResourceHandler));
    m_pShaderHandler = static_cast<ShaderHandler*>(getComponentHandler(tid_shaderHandler));
    m_pUIHandler = static_cast<UIHandler*>(getComponentHandler(tid_UIHandler));
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

    HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, m_PerPanelBuffer.GetAddressOf());
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

void UIRenderer::update(float dt)
{
    if (m_Panels.size() == 0) {
        return;
    }

    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pContext->IASetInputLayout(m_pUIProgram->inputLayout);
    UINT offsets = 0;
    m_pContext->IASetVertexBuffers(0, 1, m_Quad.GetAddressOf(), &m_pUIProgram->vertexSize, &offsets);

    m_pContext->VSSetShader(m_pUIProgram->vertexShader, nullptr, 0);
    m_pContext->HSSetShader(m_pUIProgram->hullShader, nullptr, 0);
    m_pContext->DSSetShader(m_pUIProgram->domainShader, nullptr, 0);
    m_pContext->GSSetShader(m_pUIProgram->geometryShader, nullptr, 0);
    m_pContext->PSSetShader(m_pUIProgram->pixelShader, nullptr, 0);

    m_pContext->RSSetViewports(1, &m_Viewport);

    m_pContext->PSSetSamplers(0, 1, m_ppAniSampler);
    m_pContext->OMSetRenderTargets(1, &m_pRenderTarget, m_pDepthStencilView);

    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));

    size_t bufferSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float);              // Highlight factor

    for (const Entity& entity : m_Panels.getVec()) {
        UIPanel& panel = m_pUIHandler->panels.indexID(entity);
        if (panel.texture->getSRV() == nullptr) {
            continue;
        }

        // Set per-object buffer
        m_pContext->Map(m_PerPanelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
        memcpy(mappedResources.pData, &panel, bufferSize);
        m_pContext->Unmap(m_PerPanelBuffer.Get(), 0);

        m_pContext->VSSetConstantBuffers(0, 1, m_PerPanelBuffer.GetAddressOf());
        m_pContext->PSSetConstantBuffers(0, 1, m_PerPanelBuffer.GetAddressOf());

        ID3D11ShaderResourceView* pPanelSRV = panel.texture->getSRV();
        m_pContext->PSSetShaderResources(0, 1, &pPanelSRV);

        m_pContext->Draw(4, 0);
    }
}

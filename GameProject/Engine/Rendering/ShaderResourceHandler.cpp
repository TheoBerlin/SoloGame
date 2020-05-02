#include "ShaderResourceHandler.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

ShaderResourceHandler::ShaderResourceHandler(ECSCore* pECS, IDevice* pDevice)
    :ComponentHandler(pECS, TID(ShaderResourceHandler)),
    m_pDevice(pDevice),
    m_pQuadVertices(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;

    registerHandler(handlerReg);
}

ShaderResourceHandler::~ShaderResourceHandler()
{
    delete m_pQuadVertices;
}

bool ShaderResourceHandler::initHandler()
{
    ID3D11Device* pDevice = reinterpret_cast<DeviceDX11*>(m_pDevice)->getDevice();

    /* Samplers */
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = 0.0f;

    HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, aniSampler.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create anisotropic sampler: %s", hresultToString(hr).c_str());
        return false;
    }

    // Create quad. DirectX's NDC has coordinates in [-1, 1], but here [0, 1]
    // is used as it eases resizing and positioning in the UI vertex shader.
    Vertex2D pQuadVertices[4] = {
        // Position, TXCoord
        {{0.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}}
    };

    IBuffer* pQuadBuffer = m_pDevice->createVertexBuffer(pQuadVertices, sizeof(Vertex2D), 4);
    m_pQuadVertices = reinterpret_cast<BufferDX11*>(pQuadBuffer);
    return m_pQuadVertices;
}

ID3D11SamplerState *const* ShaderResourceHandler::getAniSampler() const
{
    return aniSampler.GetAddressOf();
}

BufferDX11* ShaderResourceHandler::getQuarterScreenQuad()
{
    return m_pQuadVertices;
}

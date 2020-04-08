#include "ShaderResourceHandler.hpp"

#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

ShaderResourceHandler::ShaderResourceHandler(ECSCore* pECS, ID3D11Device* device)
    :ComponentHandler(pECS, std::type_index(typeid(ShaderResourceHandler))),
    device(device)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;

    registerHandler(handlerReg);
}

ShaderResourceHandler::~ShaderResourceHandler()
{}

bool ShaderResourceHandler::init()
{
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

    HRESULT hr = device->CreateSamplerState(&samplerDesc, aniSampler.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create anisotropic sampler: %s", hresultToString(hr).c_str());
        return false;
    }

    // Create quad. DirectX's NDC has coordinates in [-1, 1], but here [0, 1]
    // is used as it eases resizing and positioning in the UI vertex shader.
    Vertex2D quadVertices[4] = {
        // Position, txCoord
        {{0.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}}
    };

    return createVertexBuffer(quadVertices, sizeof(Vertex2D), 4, quad.GetAddressOf());
}

bool ShaderResourceHandler::createVertexBuffer(const void* vertices, size_t vertexSize, size_t vertexCount, ID3D11Buffer** targetBuffer)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = (UINT)(vertexSize * vertexCount);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    bufferData.pSysMem = vertices;
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &bufferData, targetBuffer);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create vertex buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool ShaderResourceHandler::createIndexBuffer(unsigned* indices, size_t indexCount, ID3D11Buffer** targetBuffer)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = (UINT)(sizeof(unsigned int) * indexCount);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    bufferData.pSysMem = indices;
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &bufferData, targetBuffer);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create index buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

ID3D11SamplerState *const* ShaderResourceHandler::getAniSampler() const
{
    return aniSampler.GetAddressOf();
}

Microsoft::WRL::ComPtr<ID3D11Buffer> ShaderResourceHandler::getQuarterScreenQuad()
{
    return quad;
}

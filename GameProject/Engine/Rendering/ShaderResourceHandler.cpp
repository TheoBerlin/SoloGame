#include "ShaderResourceHandler.hpp"

#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

ShaderResourceHandler::ShaderResourceHandler(SystemSubscriber* sysSubscriber, ID3D11Device* device)
    :ComponentHandler({}, sysSubscriber, std::type_index(typeid(ShaderResourceHandler))),
    device(device)
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
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create anisotropic sampler: %s", hresultToString(hr).c_str());
}

ShaderResourceHandler::~ShaderResourceHandler()
{}

void ShaderResourceHandler::createVertexBuffer(const void* vertices, size_t vertexSize, size_t vertexCount, ID3D11Buffer** targetBuffer)
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
        Logger::LOG_WARNING("Failed to create vertex buffer: %s", hresultToString(hr).c_str());
    }
}

void ShaderResourceHandler::createIndexBuffer(unsigned* indices, size_t indexCount, ID3D11Buffer** targetBuffer)
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
        Logger::LOG_WARNING("Failed to create index buffer: %s", hresultToString(hr).c_str());
    }
}

ID3D11SamplerState *const* ShaderResourceHandler::getAniSampler() const
{
    return aniSampler.GetAddressOf();
}
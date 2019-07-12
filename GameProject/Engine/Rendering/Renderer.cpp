#include "Renderer.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <DirectXMath.h>

Renderer::Renderer(ECSInterface* ecs, ID3D11Device* device)
    :System(ecs)
{
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));

    this->shaderHandler = static_cast<ShaderHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderHandler));

    SystemRegistration sysReg = {
    {
        {{{R, tid_renderable}}, &renderables}
    },
    this};

    this->subscribeToComponents(&sysReg);

    /* Shader resource creation */
    /* Cbuffers */
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4) * 2;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &perObjectMatrices);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per-object matrices cbuffer: %s", hresultToString(hr).c_str());

    bufferDesc.ByteWidth = sizeof(Material);

    hr = device->CreateBuffer(&bufferDesc, nullptr, &materialCBuffer);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create material cbuffer: %s", hresultToString(hr).c_str());

    bufferDesc.ByteWidth = sizeof(PointLight);

    hr = device->CreateBuffer(&bufferDesc, nullptr, &pointLightCBuffer);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create point light cbuffer: %s", hresultToString(hr).c_str());

    bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT3) + sizeof(int);

    hr = device->CreateBuffer(&bufferDesc, nullptr, &perFramePS);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per frame PS cbuffer: %s", hresultToString(hr).c_str());

    /* Samplers */
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = 0.0f;

    device->CreateSamplerState(&samplerDesc, &aniSampler);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create anisotropic sampler: %s", hresultToString(hr).c_str());
}

Renderer::~Renderer()
{}

void Renderer::update(float dt)
{}

#include "Renderer.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <DirectXMath.h>

Renderer::Renderer(ECSInterface* ecs, ID3D11Device* device, ID3D11DeviceContext* context)
    :System(ecs),
    context(context)
{
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    std::type_index tid_renderableHandler = std::type_index(typeid(RenderableHandler));

    this->shaderHandler = static_cast<ShaderHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderHandler));
    this->renderableHandler = static_cast<RenderableHandler*>(ecs->systemSubscriber.getComponentHandler(tid_renderableHandler));

    SystemRegistration sysReg = {
    {
        {{{R, tid_renderable}, {R, tid_worldMatrix}}, &renderables}
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

    bufferDesc.ByteWidth = sizeof(MaterialAttributes);

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
{
    for (size_t i = 0; i < renderables.size(); i += 1) {
        Model* model = renderableHandler->renderables[i].model;

        context->VSSetShader(renderableHandler->renderables[i].program->vertexShader, nullptr, 0);
        context->HSSetShader(renderableHandler->renderables[i].program->hullShader, nullptr, 0);
        context->DSSetShader(renderableHandler->renderables[i].program->domainShader, nullptr, 0);
        context->GSSetShader(renderableHandler->renderables[i].program->geometryShader, nullptr, 0);
        context->PSSetShader(renderableHandler->renderables[i].program->pixelShader, nullptr, 0);

        for (size_t j = 0; j < model->meshes.size(); j += 1) {
            context->IASetVertexBuffers(0, 1, &model->meshes[j].vertexBuffer, nullptr, nullptr);

            // Hardcode the shader resource binding for now, this cold be made more flexible when more shader programs exist
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

            /* Vertex shader */
            PerObjectMatrices matrices;
            // TODO: Declare camera component containing VP matrices, subscribe to it
            matrices.WVP = transformHandler->getWorldMatrix(renderables[i]).worldMatrix;
            matrices.world = transformHandler->getWorldMatrix(renderables[i]).worldMatrix;
            context->Map(perObjectMatrices, 0, D3D11_MAP_WRITE, 0, &mappedResource);
            //memcpy(&matrices, &mappedResource, sizeof(matrices));
            // Edit matrices
            context->Unmap(perObjectMatrices, 0);
            context->VSSetConstantBuffers(0, 1, &perObjectMatrices);
        }
    }
}

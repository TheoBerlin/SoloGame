#include "Renderer.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <DirectXMath.h>

Renderer::Renderer(ECSInterface* ecs, ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
    :System(ecs),
    context(context),
    renderTarget(rtv),
    depthStencilView(dsv)
{
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    std::type_index tid_renderableHandler = std::type_index(typeid(RenderableHandler));
    std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler));
    std::type_index tid_vpHandler = std::type_index(typeid(VPHandler));

    this->shaderHandler = static_cast<ShaderHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderHandler));
    this->renderableHandler = static_cast<RenderableHandler*>(ecs->systemSubscriber.getComponentHandler(tid_renderableHandler));
    this->transformHandler = static_cast<TransformHandler*>(ecs->systemSubscriber.getComponentHandler(tid_transformHandler));
    this->vpHandler = static_cast<VPHandler*>(ecs->systemSubscriber.getComponentHandler(tid_vpHandler));

    SystemRegistration sysReg = {
    {
        {{{R, tid_renderable}, {R, tid_worldMatrix}}, &renderables},
        {{{R, tid_transform}, {R, tid_view}, {R, tid_projection}}, &camera},
        {{{R, tid_pointLight}}, &pointLights}
    },
    this};

    this->subscribeToComponents(&sysReg);

    /* Shader resource creation */
    /* Cbuffers */
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(PerObjectMatrices);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &perObjectMatrices);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per-object matrices cbuffer: %s", hresultToString(hr).c_str());

    bufferDesc.ByteWidth = sizeof(MaterialAttributes);

    hr = device->CreateBuffer(&bufferDesc, nullptr, &materialBuffer);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create material cbuffer: %s", hresultToString(hr).c_str());

    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);

    hr = device->CreateBuffer(&bufferDesc, nullptr, &pointLightBuffer);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create point light cbuffer: %s", hresultToString(hr).c_str());

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

    /* Rasterizer state */
    D3D11_RASTERIZER_DESC rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthBias = 0;
    rsDesc.SlopeScaledDepthBias = 0.0f;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.DepthClipEnable = true;
    rsDesc.ScissorEnable = false;
    rsDesc.MultisampleEnable = false;
    rsDesc.AntialiasedLineEnable = false;

    hr = device->CreateRasterizerState(&rsDesc, &rsState);
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create rasterizer state: %s", hresultToString(hr).c_str());
}

Renderer::~Renderer()
{
    if (meshInputLayout)
        meshInputLayout->Release();

    if (perObjectMatrices)
        perObjectMatrices->Release();

    if (materialBuffer)
        materialBuffer->Release();

    if (pointLightBuffer)
        pointLightBuffer->Release();

    if (aniSampler)
        aniSampler->Release();

    if (rsState)
        rsState->Release();
}

void Renderer::update(float dt)
{
    if (renderables.size() == 0 || camera.size() == 0)
       return;

    // Hardcode the shader resource binding for now, this cold be made more flexible when more shader programs exist
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Point light cbuffer
    PerFrameBuffer perFrame;
    unsigned int numLights = std::min(MAX_POINTLIGHTS, (int)pointLights.size());
    for (unsigned int i = 0; i < numLights; i += 1) {
        perFrame.pointLights[i] = lightHandler->pointLights[i];
    }

    perFrame.cameraPosition = transformHandler->transforms[camera[0]].position;
    perFrame.numLights = numLights;

    context->Map(pointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &perFrame, sizeof(PerFrameBuffer));
    context->Unmap(pointLightBuffer, 0);
    context->PSSetConstantBuffers(0, 1, &pointLightBuffer);

    context->RSSetState(rsState);
    context->OMSetRenderTargets(1, &renderTarget, depthStencilView);

    for (size_t i = 0; i < renderables.size(); i += 1) {
        Renderable& renderable = renderableHandler->renderables[i];
        Program* program = renderable.program;
        Model* model = renderable.model;

        context->VSSetShader(program->vertexShader, nullptr, 0);
        context->HSSetShader(program->hullShader, nullptr, 0);
        context->DSSetShader(program->domainShader, nullptr, 0);
        context->GSSetShader(program->geometryShader, nullptr, 0);
        context->PSSetShader(program->pixelShader, nullptr, 0);

        // Prepare camera's view*proj matrix
        ViewMatrix camView = vpHandler->viewMatrices.indexID(camera[0]);
        ProjectionMatrix camProj = vpHandler->projMatrices.indexID(camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&camView.view) * DirectX::XMLoadFloat4x4(&camProj.projection);

        for (size_t j = 0; j < model->meshes.size(); j += 1) {
            Mesh& mesh = model->meshes[j];
            if (model->materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            // Vertex buffer
            context->IASetInputLayout(program->inputLayout);
            UINT offsets = 0;
            context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &program->vertexSize, &offsets);

            /* Vertex shader */
            PerObjectMatrices matrices;

            // PerObjectMatrices cbuffer
            matrices.world = transformHandler->getWorldMatrix(renderables[i]).worldMatrix;
            DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.world) * camVP));

            ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
            context->Map(perObjectMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            memcpy(mappedResource.pData, &matrices, sizeof(PerObjectMatrices));
            context->Unmap(perObjectMatrices, 0);
            context->VSSetConstantBuffers(0, 1, &perObjectMatrices);

            /* Pixel shader */
            // Diffuse texture
            context->PSSetShaderResources(0, 1, &model->materials[mesh.materialIndex].textures[0].srv);
            context->PSSetSamplers(0, 1, &this->aniSampler);

            // Material cbuffer
            ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
            context->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            memcpy(mappedResource.pData, &model->materials[mesh.materialIndex].attributes, sizeof(Material));
            context->Unmap(materialBuffer, 0);
            context->PSSetConstantBuffers(0, 1, &materialBuffer);

            context->Draw((UINT)mesh.vertexCount, 0);
        }
    }
}

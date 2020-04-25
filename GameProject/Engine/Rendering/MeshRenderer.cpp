#include "MeshRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <DirectXMath.h>

MeshRenderer::MeshRenderer(ECSCore* pECS, Display* pDisplay)
    :Renderer(pECS, pDisplay->getDevice(), pDisplay->getDeviceContext()),
    m_pCommandBuffer(nullptr),
    m_pRenderTarget(pDisplay->getRenderTarget()),
    m_pDepthStencilView(pDisplay->getDepthStencilView()),
    m_BackbufferWidth(pDisplay->getClientWidth()),
    m_BackbufferHeight(pDisplay->getClientHeight())
{
    CameraComponents camSub;
    PointLightComponents pointLightSub;

    RendererRegistration rendererReg = {
        {
            {{{R, g_TIDRenderable}, {R, g_TIDWorldMatrix}}, &m_Renderables},
            {{{R, g_TIDViewProjectionMatrices}}, {&camSub}, &m_Camera},
            {{&pointLightSub}, &m_PointLights}
        },
        this
    };

    registerRenderer(rendererReg);
}

MeshRenderer::~MeshRenderer()
{
    SAFERELEASE(m_pCommandBuffer)
    SAFERELEASE(m_pMeshInputLayout)
    SAFERELEASE(m_pPerObjectMatrices)
    SAFERELEASE(m_pMaterialBuffer)
    SAFERELEASE(m_pPointLightBuffer)
    SAFERELEASE(m_RsState)
}

bool MeshRenderer::init()
{
    if (!createCommandBuffer(&m_pCommandBuffer)) {
        return false;
    }

    m_pRenderableHandler = static_cast<RenderableHandler*>(getComponentHandler(TID(RenderableHandler)));
    m_pTransformHandler = static_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pVPHandler = static_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));
    m_pLightHandler = static_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));

    if (!m_pRenderableHandler || !m_pTransformHandler || !m_pVPHandler || !m_pLightHandler) {
        return false;
    }

    // Retrieve anisotropic sampler
    std::type_index tid_shaderResourceHandler = std::type_index(typeid(ShaderResourceHandler));
    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(tid_shaderResourceHandler));

    m_ppAniSampler = pShaderResourceHandler->getAniSampler();

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

    HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pPerObjectMatrices);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create per-object matrices cbuffer: %s", hresultToString(hr).c_str());
        return false;
    }

    bufferDesc.ByteWidth = sizeof(MaterialAttributes);

    hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pMaterialBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create material cbuffer: %s", hresultToString(hr).c_str());
        return false;
    }

    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);

    hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pPointLightBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create point light cbuffer: %s", hresultToString(hr).c_str());
        return false;
    }

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

    hr = m_pDevice->CreateRasterizerState(&rsDesc, &m_RsState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create rasterizer state: %s", hresultToString(hr).c_str());
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

void MeshRenderer::recordCommands()
{
    if (m_Renderables.size() == 0 || m_Camera.size() == 0) {
       return;
    }

    m_pCommandBuffer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Point light cbuffer
    PerFrameBuffer perFrame;
    uint32_t numLights = std::min(MAX_POINTLIGHTS, (uint32_t)m_PointLights.size());
    for (uint32_t i = 0; i < numLights; i += 1) {
        Entity pointLightEntity = m_PointLights[i];

        const PointLight& pointLight = m_pLightHandler->getPointLight(pointLightEntity);

        perFrame.PointLights[i] = {
            m_pTransformHandler->getPosition(pointLightEntity),
            pointLight.RadiusReciprocal,
            pointLight.Light
        };
    }

    perFrame.CameraPosition = m_pTransformHandler->getPosition(m_Camera[0]);
    perFrame.NumLights = numLights;

    // Hardcode the shader resource binding for now, this cold be made more flexible when more shader programs exist
    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));
    m_pCommandBuffer->Map(m_pPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
    memcpy(mappedResources.pData, &perFrame, sizeof(PerFrameBuffer));
    m_pCommandBuffer->Unmap(m_pPointLightBuffer, 0);
    m_pCommandBuffer->PSSetConstantBuffers(1, 1, &m_pPointLightBuffer);

    m_pCommandBuffer->RSSetState(m_RsState);
    m_pCommandBuffer->OMSetRenderTargets(1, &m_pRenderTarget, m_pDepthStencilView);

    for (Entity renderableID : m_Renderables.getIDs()) {
        Renderable& renderable = m_pRenderableHandler->m_Renderables.indexID(renderableID);
        Program* program = renderable.program;
        Model* model = renderable.model;

        m_pCommandBuffer->VSSetShader(program->vertexShader, nullptr, 0);
        m_pCommandBuffer->HSSetShader(program->hullShader, nullptr, 0);
        m_pCommandBuffer->DSSetShader(program->domainShader, nullptr, 0);
        m_pCommandBuffer->GSSetShader(program->geometryShader, nullptr, 0);
        m_pCommandBuffer->PSSetShader(program->pixelShader, nullptr, 0);

        // Prepare camera's view*proj matrix
        const ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(m_Camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&vpMatrices.View) * DirectX::XMLoadFloat4x4(&vpMatrices.Projection);

        for (const Mesh& mesh : model->meshes) {
            if (model->materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            // Vertex buffer
            m_pCommandBuffer->IASetInputLayout(program->inputLayout);
            UINT offsets = 0;
            m_pCommandBuffer->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &program->vertexSize, &offsets);
            m_pCommandBuffer->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            /* Vertex shader */
            PerObjectMatrices matrices;

            // PerObjectMatrices cbuffer
            matrices.World = m_pTransformHandler->getWorldMatrix(renderableID).worldMatrix;
            DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World) * camVP));

            m_pCommandBuffer->Map(m_pPerObjectMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
            memcpy(mappedResources.pData, &matrices, sizeof(PerObjectMatrices));
            m_pCommandBuffer->Unmap(m_pPerObjectMatrices, 0);
            m_pCommandBuffer->VSSetConstantBuffers(0, 1, &m_pPerObjectMatrices);

            m_pCommandBuffer->RSSetViewports(1, &m_Viewport);

            /* Pixel shader */
            // Diffuse texture
            ID3D11ShaderResourceView* pDiffuseSRV = model->materials[mesh.materialIndex].textures[0].getSRV();
            m_pCommandBuffer->PSSetShaderResources(0, 1, &pDiffuseSRV);
            m_pCommandBuffer->PSSetSamplers(0, 1, m_ppAniSampler);

            // Material cbuffer
            m_pCommandBuffer->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
            memcpy(mappedResources.pData, &model->materials[mesh.materialIndex].attributes, sizeof(MaterialAttributes));
            m_pCommandBuffer->Unmap(m_pMaterialBuffer, 0);
            m_pCommandBuffer->PSSetConstantBuffers(0, 1, &m_pMaterialBuffer);

            m_pCommandBuffer->DrawIndexed((UINT)mesh.indexCount, 0, 0);
        }
    }
}

bool MeshRenderer::executeCommands()
{
    return executeCommandBuffer(m_pCommandBuffer);
}

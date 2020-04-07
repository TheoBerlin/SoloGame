#include "Renderer.hpp"

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <DirectXMath.h>

MeshRenderer::MeshRenderer(ECSCore* pECS, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
    :System(pECS),
    m_pDevice(pDevice),
    m_pContext(pContext),
    m_pRenderTarget(pRTV),
    m_pDepthStencilView(pDSV)
{

    SystemRegistration sysReg = {
    {
        {{{R, tid_renderable}, {R, tid_worldMatrix}}, &m_Renderables},
        {{{R, tid_transform}, {R, tid_view}, {R, tid_projection}}, &m_Camera},
        {{{R, tid_pointLight}}, &m_PointLights}
    },
    this};

    subscribeToComponents(sysReg);
}

MeshRenderer::~MeshRenderer()
{
    if (m_pMeshInputLayout) {
        m_pMeshInputLayout->Release();
    }

    if (m_pPerObjectMatrices) {
        m_pPerObjectMatrices->Release();
    }

    if (m_pMaterialBuffer) {
        m_pMaterialBuffer->Release();
    }

    if (m_pPointLightBuffer) {
        m_pPointLightBuffer->Release();
    }

    if (m_RsState) {
        m_RsState->Release();
    }
}

bool MeshRenderer::init()
{
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

    return true;
}

void MeshRenderer::update(float dt)
{
    if (m_Renderables.size() == 0 || m_Camera.size() == 0)
       return;

    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Point light cbuffer
    PerFrameBuffer perFrame;
    uint32_t numLights = std::min(MAX_POINTLIGHTS, (int)m_PointLights.size());
    for (unsigned int i = 0; i < numLights; i += 1) {
        perFrame.pointLights[i] = m_pLightHandler->pointLights[i];
    }

    perFrame.cameraPosition = m_pTransformHandler->transforms[m_Camera[0]].position;
    perFrame.numLights = numLights;

    // Hardcode the shader resource binding for now, this cold be made more flexible when more shader programs exist
    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));
    m_pContext->Map(m_pPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
    memcpy(mappedResources.pData, &perFrame, sizeof(PerFrameBuffer));
    m_pContext->Unmap(m_pPointLightBuffer, 0);
    m_pContext->PSSetConstantBuffers(1, 1, &m_pPointLightBuffer);

    m_pContext->RSSetState(m_RsState);
    m_pContext->OMSetRenderTargets(1, &m_pRenderTarget, m_pDepthStencilView);

    for (size_t i = 0; i < m_Renderables.size(); i += 1) {
        Renderable& renderable = m_pRenderableHandler->m_Renderables[i];
        Program* program = renderable.program;
        Model* model = renderable.model;

        m_pContext->VSSetShader(program->vertexShader, nullptr, 0);
        m_pContext->HSSetShader(program->hullShader, nullptr, 0);
        m_pContext->DSSetShader(program->domainShader, nullptr, 0);
        m_pContext->GSSetShader(program->geometryShader, nullptr, 0);
        m_pContext->PSSetShader(program->pixelShader, nullptr, 0);

        // Prepare camera's view*proj matrix
        ViewMatrix camView = m_pVPHandler->viewMatrices.indexID(m_Camera[0]);
        ProjectionMatrix camProj = m_pVPHandler->projMatrices.indexID(m_Camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&camView.view) * DirectX::XMLoadFloat4x4(&camProj.projection);

        for (size_t j = 0; j < model->meshes.size(); j += 1) {
            Mesh& mesh = model->meshes[j];
            if (model->materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            // Vertex buffer
            m_pContext->IASetInputLayout(program->inputLayout);
            UINT offsets = 0;
            m_pContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &program->vertexSize, &offsets);
            m_pContext->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            /* Vertex shader */
            PerObjectMatrices matrices;

            // PerObjectMatrices cbuffer
            matrices.world = m_pTransformHandler->getWorldMatrix(m_Renderables[i]).worldMatrix;
            DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.world) * camVP));

            m_pContext->Map(m_pPerObjectMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
            memcpy(mappedResources.pData, &matrices, sizeof(PerObjectMatrices));
            m_pContext->Unmap(m_pPerObjectMatrices, 0);
            m_pContext->VSSetConstantBuffers(0, 1, &m_pPerObjectMatrices);

            /* Pixel shader */
            // Diffuse texture
            ID3D11ShaderResourceView* pDiffuseSRV = model->materials[mesh.materialIndex].textures[0].getSRV();
            m_pContext->PSSetShaderResources(0, 1, &pDiffuseSRV);
            m_pContext->PSSetSamplers(0, 1, m_ppAniSampler);

            // Material cbuffer
            m_pContext->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
            memcpy(mappedResources.pData, &model->materials[mesh.materialIndex].attributes, sizeof(MaterialAttributes));
            m_pContext->Unmap(m_pMaterialBuffer, 0);
            m_pContext->PSSetConstantBuffers(0, 1, &m_pMaterialBuffer);

            m_pContext->DrawIndexed((UINT)mesh.indexCount, 0, 0);
        }
    }
}

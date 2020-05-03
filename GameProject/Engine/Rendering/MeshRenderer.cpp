#include "MeshRenderer.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <DirectXMath.h>

MeshRenderer::MeshRenderer(ECSCore* pECS, DeviceDX11* pDevice, Window* pWindow)
    :Renderer(pECS, pDevice),
    m_pCommandList(nullptr),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencilView(pDevice->getDepthStencilView()),
    m_BackbufferWidth(pWindow->getWidth()),
    m_BackbufferHeight(pWindow->getHeight())
{
    CameraComponents camSub;
    PointLightComponents pointLightSub;

    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, g_TIDRenderable}, {R, g_TIDWorldMatrix}}, &m_Renderables},
        {{{R, g_TIDViewProjectionMatrices}}, {&camSub}, &m_Camera},
        {{&pointLightSub}, &m_PointLights}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

MeshRenderer::~MeshRenderer()
{
    delete m_pPerObjectMatrices;
    delete m_pMaterialBuffer;
    delete m_pPointLightBuffer;
    delete m_pCommandList;

    SAFERELEASE(m_pMeshInputLayout)
    SAFERELEASE(m_RsState)
}

bool MeshRenderer::init()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
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
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize     = sizeof(PerObjectMatrices);
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage        = BUFFER_USAGE::UNIFORM_BUFFER;

    m_pPerObjectMatrices = m_pDevice->createBuffer(bufferInfo);
    if (!m_pPerObjectMatrices) {
        LOG_ERROR("Failed to create per-object matrices uniform buffer");
        return false;
    }

    bufferInfo.ByteSize = sizeof(MaterialAttributes);
    m_pMaterialBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!m_pMaterialBuffer) {
        LOG_ERROR("Failed to create material uniform buffer");
        return false;
    }

    bufferInfo.ByteSize = sizeof(PerFrameBuffer);
    m_pPointLightBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!m_pPointLightBuffer) {
        LOG_ERROR("Failed to create point light uniform buffer");
        return false;
    }

    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(m_pDevice);
    ID3D11Device* pDevice = pDeviceDX->getDevice();

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

    HRESULT hr = pDevice->CreateRasterizerState(&rsDesc, &m_RsState);
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

    ID3D11DeviceContext* pContext = reinterpret_cast<CommandListDX11*>(m_pCommandList)->getContext();

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

    void* pMappedMemory = nullptr;
    m_pCommandList->map(m_pPointLightBuffer, &pMappedMemory);
    memcpy(pMappedMemory, &perFrame, sizeof(PerFrameBuffer));
    m_pCommandList->unmap(m_pPointLightBuffer);

    m_pCommandList->bindBuffer(1, SHADER_TYPE::FRAGMENT_SHADER, m_pPointLightBuffer);

    pContext->RSSetState(m_RsState);
    pContext->OMSetRenderTargets(1, &m_pRenderTarget, m_pDepthStencilView);

    for (Entity renderableID : m_Renderables.getIDs()) {
        Renderable& renderable = m_pRenderableHandler->m_Renderables.indexID(renderableID);
        Program* pProgram = renderable.program;
        Model* model = renderable.model;

        m_pCommandList->bindShaders(pProgram);

        // Prepare camera's view*proj matrix
        const ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(m_Camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&vpMatrices.View) * DirectX::XMLoadFloat4x4(&vpMatrices.Projection);

        for (const Mesh& mesh : model->Meshes) {
            if (model->Materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            // Vertex buffer
            pContext->IASetInputLayout(pProgram->inputLayout);

            m_pCommandList->bindVertexBuffer(0, (size_t)pProgram->vertexSize, mesh.pVertexBuffer);
            m_pCommandList->bindIndexBuffer(mesh.pIndexBuffer);

            /* Vertex shader */
            PerObjectMatrices matrices;

            // PerObjectMatrices cbuffer
            matrices.World = m_pTransformHandler->getWorldMatrix(renderableID).worldMatrix;
            DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World) * camVP));

            pMappedMemory = nullptr;
            m_pCommandList->map(m_pPerObjectMatrices, &pMappedMemory);
            memcpy(pMappedMemory, &matrices, sizeof(PerObjectMatrices));
            m_pCommandList->unmap(m_pPerObjectMatrices);

            m_pCommandList->bindBuffer(0, SHADER_TYPE::VERTEX_SHADER, m_pPerObjectMatrices);

            pContext->RSSetViewports(1, &m_Viewport);

            /* Pixel shader */
            // Diffuse texture
            ID3D11ShaderResourceView* pDiffuseSRV = model->Materials[mesh.materialIndex].textures[0].getSRV();
            pContext->PSSetShaderResources(0, 1, &pDiffuseSRV);
            pContext->PSSetSamplers(0, 1, m_ppAniSampler);

            // Material cbuffer
            pMappedMemory = nullptr;
            m_pCommandList->map(m_pMaterialBuffer, &pMappedMemory);
            memcpy(pMappedMemory, &model->Materials[mesh.materialIndex].attributes, sizeof(MaterialAttributes));
            m_pCommandList->unmap(m_pMaterialBuffer);

            m_pCommandList->bindBuffer(0, SHADER_TYPE::FRAGMENT_SHADER, m_pMaterialBuffer);

            m_pCommandList->drawIndexed(mesh.indexCount);
        }
    }
}

void MeshRenderer::executeCommands()
{
    m_pCommandList->execute();
}

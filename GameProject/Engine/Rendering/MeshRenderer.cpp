#include "MeshRenderer.hpp"

#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderBindings.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/Logger.hpp>

MeshRenderer::MeshRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :Renderer(pECS, pDevice),
    m_pCommandList(nullptr),
    m_pRenderableHandler(nullptr),
    m_pTransformHandler(nullptr),
    m_pVPHandler(nullptr),
    m_pLightHandler(nullptr),
    m_pDescriptorSetLayoutCommon(nullptr),
    m_pDescriptorSetLayoutModel(nullptr),
    m_pDescriptorSetLayoutMesh(nullptr),
    m_pDescriptorSetCommon(nullptr),
    m_pRasterizerState(nullptr),
    m_pAniSampler(nullptr),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencil(pDevice->getDepthStencil()),
    m_BackbufferWidth(pWindow->getWidth()),
    m_BackbufferHeight(pWindow->getHeight()),
    m_pDepthStencilState(nullptr)
{
    CameraComponents camSub;
    PointLightComponents pointLightSub;

    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, g_TIDRenderable}, {R, g_TIDWorldMatrix}}, &m_Renderables, [this](Entity entity){ onMeshAdded(entity); }, [this](Entity entity){ onMeshRemoved(entity); }},
        {{{R, g_TIDViewProjectionMatrices}}, {&camSub}, &m_Camera},
        {{&pointLightSub}, &m_PointLights}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

MeshRenderer::~MeshRenderer()
{
    delete m_pPointLightBuffer;
    delete m_pCommandList;
    delete m_pRasterizerState;
    delete m_pDepthStencilState;
    delete m_pDescriptorSetLayoutCommon;
    delete m_pDescriptorSetLayoutModel;
    delete m_pDescriptorSetLayoutMesh;
    delete m_pDescriptorSetCommon;

    // Delete all model rendering resources
    for (Entity entity : m_ModelRenderResources.getIDs()) {
        onMeshRemoved(entity);
    }
}

bool MeshRenderer::init()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
        return false;
    }

    m_pRenderableHandler    = static_cast<RenderableHandler*>(getComponentHandler(TID(RenderableHandler)));
    m_pTransformHandler     = static_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pVPHandler            = static_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));
    m_pLightHandler         = static_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));

    if (!m_pRenderableHandler || !m_pTransformHandler || !m_pVPHandler || !m_pLightHandler) {
        return false;
    }

    // Retrieve anisotropic sampler
    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));

    m_pAniSampler = pShaderResourceHandler->getAniSampler();

    if (!createBuffers()) {
        return false;
    }

    if (!createDescriptorSetLayouts()) {
        return false;
    }

    if (!createCommonDescriptorSet()) {
        return false;
    }

    // Rasterizer state
    RasterizerStateInfo rasterizerInfo = {};
    rasterizerInfo.PolygonMode          = POLYGON_MODE::FILL;
    rasterizerInfo.CullMode             = CULL_MODE::BACK;
    rasterizerInfo.FrontFaceOrientation = FRONT_FACE_ORIENTATION::CLOCKWISE;
    rasterizerInfo.DepthBiasEnable      = false;

    m_pRasterizerState = m_pDevice->createRasterizerState(rasterizerInfo);
    if (!m_pRasterizerState) {
        return false;
    }

    DepthStencilInfo depthStencilInfo = {};
    depthStencilInfo.DepthTestEnabled = true;
    depthStencilInfo.DepthWriteEnabled = true;
    depthStencilInfo.DepthComparisonFunc = COMPARISON_FUNC::LESS;
    depthStencilInfo.StencilTestEnabled = false;

    m_pDepthStencilState = m_pDevice->createDepthStencilState(depthStencilInfo);
    if (!m_pDepthStencilState) {
        return false;
    }

    // Create viewport
    m_Viewport = {};
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width    = (float)m_BackbufferWidth;
    m_Viewport.Height   = (float)m_BackbufferHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    return true;
}

void MeshRenderer::updateBuffers()
{
    if (m_Renderables.size() == 0 || m_Camera.size() == 0) {
       return;
    }

    // Update point light uniform buffer
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

    for (Entity renderableID : m_Renderables.getIDs()) {
        Renderable& renderable = m_pRenderableHandler->m_Renderables.indexID(renderableID);
        Model* model = renderable.model;
        ModelRenderResources& modelRenderResources = m_ModelRenderResources.indexID(renderableID);

        // Prepare camera's view*proj matrix
        const ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(m_Camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&vpMatrices.View) * DirectX::XMLoadFloat4x4(&vpMatrices.Projection);

        // Update per-object matrices uniform buffer
        PerObjectMatrices matrices;
        matrices.World = m_pTransformHandler->getWorldMatrix(renderableID).worldMatrix;
        DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World) * camVP));

        pMappedMemory = nullptr;
        m_pCommandList->map(modelRenderResources.pWVPBuffer, &pMappedMemory);
        memcpy(pMappedMemory, &matrices, sizeof(PerObjectMatrices));
        m_pCommandList->unmap(modelRenderResources.pWVPBuffer);
    }
}

void MeshRenderer::recordCommands()
{
    if (m_Renderables.size() == 0 || m_Camera.size() == 0) {
       return;
    }

    m_pCommandList->bindPrimitiveTopology(PRIMITIVE_TOPOLOGY::TRIANGLE_LIST);

    m_pCommandList->bindRasterizerState(m_pRasterizerState);
    m_pCommandList->bindRenderTarget(m_pRenderTarget, m_pDepthStencil);

    m_pCommandList->bindDepthStencilState(m_pDepthStencilState);
    m_pCommandList->bindViewport(&m_Viewport);

    m_pCommandList->bindDescriptorSet(m_pDescriptorSetCommon);

    for (Entity renderableID : m_Renderables.getIDs()) {
        Renderable& renderable = m_pRenderableHandler->m_Renderables.indexID(renderableID);
        Program* pProgram = renderable.program;
        Model* pModel = renderable.model;
        const ModelRenderResources& modelRenderResources = m_ModelRenderResources.indexID(renderableID);

        m_pCommandList->bindShaders(pProgram);
        m_pCommandList->bindInputLayout(pProgram->pInputLayout);

        m_pCommandList->bindDescriptorSet(modelRenderResources.pDescriptorSet);

        size_t meshIdx = 0;

        for (const Mesh& mesh : pModel->Meshes) {
            if (pModel->Materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            const MeshRenderResources& meshRenderResources = modelRenderResources.MeshRenderResources[meshIdx];

            m_pCommandList->bindVertexBuffer(0, pProgram->pInputLayout->getVertexSize(), mesh.pVertexBuffer);
            m_pCommandList->bindIndexBuffer(mesh.pIndexBuffer);

            m_pCommandList->bindDescriptorSet(meshRenderResources.pDescriptorSet);

            m_pCommandList->drawIndexed(mesh.indexCount);

            meshIdx += 1;
        }
    }
}

void MeshRenderer::executeCommands()
{
    m_pCommandList->execute();
}

bool MeshRenderer::createBuffers()
{
    // Uniform buffers
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize     = sizeof(PerObjectMatrices);
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage        = BUFFER_USAGE::UNIFORM_BUFFER;

    bufferInfo.ByteSize = sizeof(PerFrameBuffer);
    m_pPointLightBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!m_pPointLightBuffer) {
        LOG_ERROR("Failed to create point light uniform buffer");
    }

    return m_pPointLightBuffer;
}

bool MeshRenderer::createDescriptorSetLayouts()
{
    // Common descriptor set layout
    m_pDescriptorSetLayoutCommon = m_pDevice->createDescriptorSetLayout();

    m_pDescriptorSetLayoutCommon->addBindingUniformBuffer(SHADER_BINDING::PER_FRAME, SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayoutCommon->addBindingSampler(SHADER_BINDING::SAMPLER_ONE, SHADER_TYPE::FRAGMENT_SHADER);

    if (!m_pDescriptorSetLayoutCommon->finalize()) {
        return false;
    }

    // Per-model descriptor set layout
    m_pDescriptorSetLayoutModel = m_pDevice->createDescriptorSetLayout();

    m_pDescriptorSetLayoutModel->addBindingUniformBuffer(SHADER_BINDING::PER_OBJECT, SHADER_TYPE::VERTEX_SHADER);

    if (!m_pDescriptorSetLayoutModel->finalize()) {
        return false;
    }

    // Per-mesh descriptor set layout
    m_pDescriptorSetLayoutMesh = m_pDevice->createDescriptorSetLayout();

    m_pDescriptorSetLayoutMesh->addBindingUniformBuffer(SHADER_BINDING::MATERIAL_CONSTANTS, SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayoutMesh->addBindingSampledTexture(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);

    return m_pDescriptorSetLayoutMesh->finalize();
}

bool MeshRenderer::createCommonDescriptorSet()
{
    m_pDescriptorSetCommon = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutCommon);
    if (!m_pDescriptorSetCommon) {
        return false;
    }

    m_pDescriptorSetCommon->writeUniformBufferDescriptor(SHADER_BINDING::PER_FRAME, m_pPointLightBuffer);
    m_pDescriptorSetCommon->writeSamplerDescriptor(SHADER_BINDING::SAMPLER_ONE, m_pAniSampler);
    return true;
}

void MeshRenderer::onMeshAdded(Entity entity)
{
    Renderable& renderable = m_pRenderableHandler->m_Renderables.indexID(entity);
    Program* pProgram = renderable.program;
    Model* pModel = renderable.model;

    // Create buffers and descriptor sets for the model and its meshes
    ModelRenderResources modelRenderResources = {};
    modelRenderResources.MeshRenderResources.reserve(pModel->Meshes.size());

    BufferInfo bufferInfo   = {};
    bufferInfo.ByteSize     = sizeof(PerObjectMatrices);
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage        = BUFFER_USAGE::UNIFORM_BUFFER;

    modelRenderResources.pWVPBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!modelRenderResources.pWVPBuffer) {
        LOG_ERROR("Failed to create per-object matrices uniform buffer");
        return;
    }

    // Per-model descriptor set
    modelRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutModel);
    modelRenderResources.pDescriptorSet->writeUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, modelRenderResources.pWVPBuffer);

    // Per-mesh resources
    for (const Mesh& mesh : pModel->Meshes) {
        MeshRenderResources meshRenderResources = {};

        // Create material attributes uniform buffer
        const Material& material = pModel->Materials[mesh.materialIndex];
        bufferInfo.ByteSize = sizeof(MaterialAttributes);
        bufferInfo.pData    = &material.attributes;

        meshRenderResources.pMaterialBuffer = m_pDevice->createBuffer(bufferInfo);
        if (!meshRenderResources.pMaterialBuffer) {
            LOG_ERROR("Failed to create material uniform buffer");
            return;
        }

        // Create per-mesh descriptor set
        meshRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutMesh);
        meshRenderResources.pDescriptorSet->writeUniformBufferDescriptor(SHADER_BINDING::MATERIAL_CONSTANTS, meshRenderResources.pMaterialBuffer);
        meshRenderResources.pDescriptorSet->writeSampledTextureDescriptor(SHADER_BINDING::TEXTURE_ONE, material.textures[0].get());

        modelRenderResources.MeshRenderResources.push_back(meshRenderResources);
    }

    m_ModelRenderResources.push_back(modelRenderResources, entity);
}

void MeshRenderer::onMeshRemoved(Entity entity)
{
    ModelRenderResources& modelRenderResources = m_ModelRenderResources.indexID(entity);
    delete modelRenderResources.pDescriptorSet;
    delete modelRenderResources.pWVPBuffer;

    for (MeshRenderResources& meshRenderResources : modelRenderResources.MeshRenderResources) {
        delete meshRenderResources.pDescriptorSet;
        delete meshRenderResources.pMaterialBuffer;
    }

    m_ModelRenderResources.pop(entity);
}

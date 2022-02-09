#include "MeshRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/Rendering/ShaderBindings.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Transform.hpp>

#include <algorithm>

MeshRenderer::MeshRenderer(Device* pDevice, RenderingHandler* pRenderingHandler)
    :Renderer(pDevice, pRenderingHandler),
    m_pDevice(pDevice),
    m_CommandListsToReset(MAX_FRAMES_IN_FLIGHT),
    m_pDescriptorSetLayoutCommon(nullptr),
    m_pDescriptorSetLayoutModel(nullptr),
    m_pDescriptorSetLayoutMesh(nullptr),
    m_pDescriptorSetCommon(nullptr),
    m_pAniSampler(nullptr),
    m_pRenderPass(nullptr),
    m_pPipeline(nullptr),
    m_pPipelineLayout(nullptr)
{
    std::fill_n(m_ppCommandPools, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppCommandLists, MAX_FRAMES_IN_FLIGHT, nullptr);
    std::fill_n(m_ppFramebuffers, MAX_FRAMES_IN_FLIGHT, nullptr);

    EntitySubscriberRegistration entitySubscriberRegistration = {
        {
            {
                .pSubscriber = &m_Renderables,
                .ComponentAccesses =
                {
                    { R, ModelComponent::Type() }, { R, WorldMatrixComponent::Type() }
                },
                .OnEntityAdded = std::bind_front(&MeshRenderer::OnMeshAdded, this),
                .OnEntityRemoval = std::bind_front(&MeshRenderer::OnMeshRemoved, this)
            },
            {
                .pSubscriber = &m_Camera,
                .ComponentAccesses =
                {
                    { R, PositionComponent::Type() }, { R, ViewProjectionMatricesComponent::Type() }
                },
            },
            {
                .pSubscriber = &m_PointLights,
                .ComponentAccesses =
                {
                    { R, PositionComponent::Type() }, { R, PointLightComponent::Type() }
                }
            }
        }
    };

    RegisterRenderer(entitySubscriberRegistration);
}

MeshRenderer::~MeshRenderer()
{
    // Delete all model rendering resources
    for (Entity entity : m_ModelRenderResources.GetIDs()) {
        OnMeshRemoved(entity);
    }

    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppCommandLists[frameIndex];
        delete m_ppCommandPools[frameIndex];
        delete m_ppFramebuffers[frameIndex];
    }

    delete m_pRenderPass;
    delete m_pPointLightBuffer;
    delete m_pDescriptorSetCommon;
    delete m_pDescriptorSetLayoutCommon;
    delete m_pDescriptorSetLayoutModel;
    delete m_pDescriptorSetLayoutMesh;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool MeshRenderer::Init()
{
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        m_ppCommandPools[frameIndex] = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
        if (!m_ppCommandPools[frameIndex]) {
            return false;
        }

        m_ppCommandPools[frameIndex]->allocateCommandLists(&m_ppCommandLists[frameIndex], 1u, COMMAND_LIST_LEVEL::SECONDARY);
        if (!m_ppCommandLists[frameIndex]) {
            return false;
        }
    }

    m_pAniSampler = ShaderResourceHandler::GetInstance()->GetAniSampler();

    if (!createBuffers()) {
        return false;
    }

    if (!createDescriptorSetLayouts()) {
        return false;
    }

    if (!createCommonDescriptorSet()) {
        return false;
    }

    if (!createRenderPass()) {
        return false;
    }

    if (!createFramebuffers()) {
        return false;
    }

    return createPipeline();
}

void MeshRenderer::UpdateBuffers()
{
    if (m_Renderables.Empty() || m_Camera.Empty()) {
       return;
    }

    ECSCore* pECS = ECSCore::GetInstance();
    const ComponentArray<PointLightComponent>* pPointLightComponents = pECS->GetComponentArray<PointLightComponent>();
    const ComponentArray<PositionComponent>* pPositionComponents = pECS->GetComponentArray<PositionComponent>();

    // Update point light uniform buffer
    PerFrameBuffer perFrame;
    const uint32_t numLights = std::min(MAX_POINTLIGHTS, m_PointLights.Size());
    for (uint32_t i = 0; i < numLights; i += 1) {
        const Entity pointLightEntity = m_PointLights[i];

        const PointLightComponent& pointLight = pPointLightComponents->GetConstData(pointLightEntity);
        const DirectX::XMFLOAT3& pointLightPos = pPositionComponents->GetConstData(pointLightEntity).Position;

        perFrame.PointLights[i] = {
            .Position = pointLightPos,
            .Light = pointLight.Light,
            .RadiusReciprocal = pointLight.RadiusReciprocal
        };
    }

    const Entity cameraEntity = m_Camera[0];
    perFrame.CameraPosition = pPositionComponents->GetConstData(cameraEntity).Position;
    perFrame.NumLights = numLights;

    void* pMappedMemory = nullptr;
    m_pDevice->map(m_pPointLightBuffer, &pMappedMemory);
    memcpy(pMappedMemory, &perFrame, sizeof(PerFrameBuffer));
    m_pDevice->unmap(m_pPointLightBuffer);

    const ComponentArray<ViewProjectionMatricesComponent>* pVPMatricesComponents = pECS->GetComponentArray<ViewProjectionMatricesComponent>();
    const ComponentArray<WorldMatrixComponent>* pWorldMatrixComponents = pECS->GetComponentArray<WorldMatrixComponent>();

    for (Entity renderableEntity : m_Renderables) {
        ModelRenderResources& modelRenderResources = m_ModelRenderResources.IndexID(renderableEntity);

        // Prepare camera's view*proj matrix
        const ViewProjectionMatricesComponent& vpMatrices = pVPMatricesComponents->GetConstData(cameraEntity);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&vpMatrices.View) * DirectX::XMLoadFloat4x4(&vpMatrices.Projection);

        // Update per-object matrices uniform buffer
        PerObjectMatrices matrices;
        matrices.World = pWorldMatrixComponents->GetConstData(renderableEntity).WorldMatrix;
        DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World) * camVP));
        DirectX::XMStoreFloat4x4(&matrices.World, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World)));

        pMappedMemory = nullptr;
        m_pDevice->map(modelRenderResources.pWVPBuffer, &pMappedMemory);
        memcpy(pMappedMemory, &matrices, sizeof(PerObjectMatrices));
        m_pDevice->unmap(modelRenderResources.pWVPBuffer);
    }
}

void MeshRenderer::RecordCommands()
{
    if (m_CommandListsToReset == 0u) {
        return;
    }

    m_CommandListsToReset -= 1u;

    const uint32_t frameIndex = m_pDevice->getFrameIndex();
    ICommandList* pCommandList = m_ppCommandLists[frameIndex];

    CommandListBeginInfo beginInfo = {};
    beginInfo.pRenderPass   = m_pRenderPass;
    beginInfo.Subpass       = 0u;
    beginInfo.pFramebuffer  = m_ppFramebuffers[frameIndex];
    pCommandList->begin(COMMAND_LIST_USAGE::WITHIN_RENDER_PASS, &beginInfo);

    if (m_Renderables.Empty() || m_Camera.Empty()) {
        pCommandList->end();
        return;
    }

    pCommandList->bindPipeline(m_pPipeline);
    pCommandList->bindDescriptorSet(m_pDescriptorSetCommon, m_pPipelineLayout, 0u);

    ECSCore* pECS = ECSCore::GetInstance();
    const ComponentArray<ModelComponent>* pModelComponents = pECS->GetComponentArray<ModelComponent>();

    for (Entity renderableEntity : m_Renderables) {
        const ModelComponent& modelComp         = pModelComponents->GetConstData(renderableEntity);
        const Model* pModel                     = modelComp.ModelPtr.get();
        const std::vector<Mesh>& meshes         = pModel->Meshes;
        const std::vector<Material>& materials  = pModel->Materials;

        const ModelRenderResources& modelRenderResources = m_ModelRenderResources.IndexID(renderableEntity);

        pCommandList->bindDescriptorSet(modelRenderResources.pDescriptorSet, m_pPipelineLayout, 1u);

        size_t meshIdx = 0;
        for (const Mesh& mesh : meshes) {
            if (materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            const MeshRenderResources& meshRenderResources = modelRenderResources.MeshRenderResources[meshIdx++];

            pCommandList->bindVertexBuffer(0, mesh.pVertexBuffer);
            pCommandList->bindIndexBuffer(mesh.pIndexBuffer);

            pCommandList->bindDescriptorSet(meshRenderResources.pDescriptorSet, m_pPipelineLayout, 2u);

            pCommandList->drawIndexed(mesh.indexCount);
        }
    }

    pCommandList->end();
}

void MeshRenderer::ExecuteCommands(ICommandList* pPrimaryCommandList)
{
    pPrimaryCommandList->executeSecondaryCommandList(m_ppCommandLists[m_pDevice->getFrameIndex()]);
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

    if (!m_pDescriptorSetLayoutCommon->finalize(m_pDevice)) {
        return false;
    }

    // Per-model descriptor set layout
    m_pDescriptorSetLayoutModel = m_pDevice->createDescriptorSetLayout();

    m_pDescriptorSetLayoutModel->addBindingUniformBuffer(SHADER_BINDING::PER_OBJECT, SHADER_TYPE::VERTEX_SHADER);

    if (!m_pDescriptorSetLayoutModel->finalize(m_pDevice)) {
        return false;
    }

    // Per-mesh descriptor set layout
    m_pDescriptorSetLayoutMesh = m_pDevice->createDescriptorSetLayout();

    m_pDescriptorSetLayoutMesh->addBindingUniformBuffer(SHADER_BINDING::MATERIAL_CONSTANTS, SHADER_TYPE::FRAGMENT_SHADER);
    m_pDescriptorSetLayoutMesh->addBindingCombinedTextureSampler(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);

    return m_pDescriptorSetLayoutMesh->finalize(m_pDevice);
}

bool MeshRenderer::createCommonDescriptorSet()
{
    m_pDescriptorSetCommon = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutCommon);
    if (!m_pDescriptorSetCommon) {
        return false;
    }

    m_pDescriptorSetCommon->updateUniformBufferDescriptor(SHADER_BINDING::PER_FRAME, m_pPointLightBuffer);
    return true;
}

bool MeshRenderer::createRenderPass()
{
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    Texture* pBackbuffer = m_pDevice->getBackbuffer(0u);
    AttachmentInfo backBufferAttachment   = {};
    backBufferAttachment.Format           = pBackbuffer->getFormat();
    backBufferAttachment.Samples          = 1u;
    backBufferAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    backBufferAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    backBufferAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    backBufferAttachment.FinalLayout      = TEXTURE_LAYOUT::RENDER_TARGET;

    Texture* pDepthStencil = m_pDevice->getDepthStencil(0u);
    AttachmentInfo depthStencilAttachment = {};
    depthStencilAttachment.Format           = pDepthStencil->getFormat();
    depthStencilAttachment.Samples          = 1u;
    depthStencilAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    depthStencilAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    depthStencilAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    depthStencilAttachment.FinalLayout      = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    // Subpass
    SubpassInfo subpass = {};
    AttachmentReference backbufferRef   = {};
    backbufferRef.AttachmentIndex       = 0;
    backbufferRef.Layout                = TEXTURE_LAYOUT::RENDER_TARGET;

    AttachmentReference depthStencilRef = {};
    depthStencilRef.AttachmentIndex     = 1;
    depthStencilRef.Layout              = TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT;

    subpass.ColorAttachments        = { backbufferRef };
    subpass.pDepthStencilAttachment = &depthStencilRef;
    subpass.PipelineBindPoint       = PIPELINE_BIND_POINT::GRAPHICS;

    // Subpass dependency
    SubpassDependency subpassDependency = {};
    subpassDependency.SrcSubpass        = SUBPASS_EXTERNAL;
    subpassDependency.DstSubpass        = 0;
    subpassDependency.SrcStage          = PIPELINE_STAGE::BOTTOM_OF_PIPE;
    subpassDependency.DstStage          = PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT;
    subpassDependency.SrcAccessMask     = RESOURCE_ACCESS::MEMORY_READ;
    subpassDependency.DstAccessMask     = RESOURCE_ACCESS::COLOR_ATTACHMENT_READ | RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE;
    subpassDependency.DependencyFlags   = DEPENDENCY_FLAG::BY_REGION;

    renderPassInfo.AttachmentInfos  = { backBufferAttachment, depthStencilAttachment };
    renderPassInfo.Subpasses        = { subpass };
    renderPassInfo.Dependencies     = { subpassDependency };

    m_pRenderPass = m_pDevice->createRenderPass(renderPassInfo);
    return m_pRenderPass;
}

bool MeshRenderer::createFramebuffers()
{
    FramebufferInfo framebufferInfo = {};
    framebufferInfo.pRenderPass = m_pRenderPass;
    framebufferInfo.Dimensions  = m_pDevice->getBackbuffer(0u)->getDimensions();

    for (uint32_t framebufferIdx = 0u; framebufferIdx < MAX_FRAMES_IN_FLIGHT; framebufferIdx += 1u) {
        framebufferInfo.Attachments = { m_pDevice->getBackbuffer(framebufferIdx), m_pDevice->getDepthStencil(framebufferIdx) };
        m_ppFramebuffers[framebufferIdx] = m_pDevice->createFramebuffer(framebufferInfo);

        if (!m_ppFramebuffers[framebufferIdx]) {
            return false;
        }
    }

    return true;
}

bool MeshRenderer::createPipeline()
{
    m_pPipelineLayout = m_pDevice->createPipelineLayout({ m_pDescriptorSetLayoutCommon, m_pDescriptorSetLayoutModel, m_pDescriptorSetLayoutMesh });
    if (!m_pPipelineLayout) {
        return false;
    }

    PipelineInfo pipelineInfo = {};
    pipelineInfo.ShaderInfos = {
        {"Mesh", SHADER_TYPE::VERTEX_SHADER},
        {"Mesh", SHADER_TYPE::FRAGMENT_SHADER}
    };

    pipelineInfo.PrimitiveTopology = PRIMITIVE_TOPOLOGY::TRIANGLE_LIST;

    const glm::uvec2& backbufferDims = m_pDevice->getBackbuffer(0u)->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = (float)backbufferDims.x;
    viewport.Height   = (float)backbufferDims.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pipelineInfo.Viewports = { viewport };

    Rectangle2D scissorRectangle = {};
    scissorRectangle.Extent = backbufferDims;
    pipelineInfo.ScissorRectangles = { scissorRectangle };

    pipelineInfo.RasterizerStateInfo = {};
    pipelineInfo.RasterizerStateInfo.PolygonMode            = POLYGON_MODE::FILL;
    pipelineInfo.RasterizerStateInfo.CullMode               = CULL_MODE::BACK;
    pipelineInfo.RasterizerStateInfo.FrontFaceOrientation   = FRONT_FACE_ORIENTATION::CLOCKWISE;
    pipelineInfo.RasterizerStateInfo.DepthBiasEnable        = false;
    pipelineInfo.RasterizerStateInfo.LineWidth              = 1.0f;

    pipelineInfo.DepthStencilStateInfo = {};
    pipelineInfo.DepthStencilStateInfo.DepthTestEnabled     = true;
    pipelineInfo.DepthStencilStateInfo.DepthWriteEnabled    = true;
    pipelineInfo.DepthStencilStateInfo.DepthComparisonFunc  = COMPARISON_FUNC::LESS;
    pipelineInfo.DepthStencilStateInfo.StencilTestEnabled   = false;

    BlendRenderTargetInfo rtvBlendInfo = {};
    rtvBlendInfo.BlendEnabled           = true;
    rtvBlendInfo.SrcColorBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstColorBlendFactor    = BLEND_FACTOR::ONE_MINUS_SRC_ALPHA;
    rtvBlendInfo.ColorBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.SrcAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.DstAlphaBlendFactor    = BLEND_FACTOR::ONE;
    rtvBlendInfo.AlphaBlendOp           = BLEND_OP::ADD;
    rtvBlendInfo.ColorWriteMask         = COLOR_WRITE_MASK::ENABLE_ALL;

    pipelineInfo.BlendStateInfo.RenderTargetBlendInfos  = { rtvBlendInfo };
    pipelineInfo.BlendStateInfo.IndependentBlendEnabled = false;
    std::fill_n(pipelineInfo.BlendStateInfo.pBlendConstants, 4u, 1.0f);

    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void MeshRenderer::OnMeshAdded(Entity entity)
{
    LOG_INFO("hello there");
    m_CommandListsToReset = MAX_FRAMES_IN_FLIGHT;

    ModelComponent& modelComp           = ECSCore::GetInstance()->GetComponent<ModelComponent>(entity);
    Model* pModel                       = modelComp.ModelPtr.get();
    std::vector<Mesh>& meshes           = pModel->Meshes;
    std::vector<Material>& materials    = pModel->Materials;

    // Create buffers and descriptor sets for the model and its meshes
    ModelRenderResources modelRenderResources = {};
    modelRenderResources.MeshRenderResources.reserve(meshes.size());

    BufferInfo bufferInfo   = {
        .ByteSize     = sizeof(PerObjectMatrices),
        .CPUAccess    = BUFFER_DATA_ACCESS::WRITE,
        .GPUAccess    = BUFFER_DATA_ACCESS::READ,
        .Usage        = BUFFER_USAGE::UNIFORM_BUFFER
    };

    modelRenderResources.pWVPBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!modelRenderResources.pWVPBuffer) {
        LOG_ERROR("Failed to create per-object matrices uniform buffer");
        return;
    }

    // Per-model descriptor set
    modelRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutModel);
    modelRenderResources.pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, modelRenderResources.pWVPBuffer);

    // Per-mesh resources
    for (const Mesh& mesh : meshes) {
        MeshRenderResources meshRenderResources = {};

        // Create material attributes uniform buffer
        const Material& material = materials[mesh.materialIndex];
        bufferInfo.ByteSize = sizeof(MaterialAttributes);
        bufferInfo.pData    = &material.attributes;

        meshRenderResources.pMaterialBuffer = m_pDevice->createBuffer(bufferInfo);
        if (!meshRenderResources.pMaterialBuffer) {
            LOG_ERROR("Failed to create material uniform buffer");
            return;
        }

        // Create per-mesh descriptor set
        meshRenderResources.pDescriptorSet = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutMesh);
        meshRenderResources.pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::MATERIAL_CONSTANTS, meshRenderResources.pMaterialBuffer);
        meshRenderResources.pDescriptorSet->updateCombinedTextureSamplerDescriptor(SHADER_BINDING::TEXTURE_ONE, material.textures[0].get(), m_pAniSampler);

        modelRenderResources.MeshRenderResources.push_back(meshRenderResources);
    }

    m_ModelRenderResources.push_back(modelRenderResources, entity);
}

void MeshRenderer::OnMeshRemoved(Entity entity)
{
    m_CommandListsToReset = MAX_FRAMES_IN_FLIGHT;

    ModelRenderResources& modelRenderResources = m_ModelRenderResources.IndexID(entity);
    delete modelRenderResources.pDescriptorSet;
    delete modelRenderResources.pWVPBuffer;

    for (MeshRenderResources& meshRenderResources : modelRenderResources.MeshRenderResources) {
        delete meshRenderResources.pDescriptorSet;
        delete meshRenderResources.pMaterialBuffer;
    }

    m_ModelRenderResources.Pop(entity);
}

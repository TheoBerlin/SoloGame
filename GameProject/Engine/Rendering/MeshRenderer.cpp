#include "MeshRenderer.hpp"

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/ShaderBindings.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Transform.hpp>

MeshRenderer::MeshRenderer(ECSCore* pECS, Device* pDevice)
    :Renderer(pECS, pDevice),
    m_pCommandPool(nullptr),
    m_pCommandList(nullptr),
    m_pDevice(pDevice),
    m_pModelLoader(nullptr),
    m_pTransformHandler(nullptr),
    m_pVPHandler(nullptr),
    m_pLightHandler(nullptr),
    m_pDescriptorSetLayoutCommon(nullptr),
    m_pDescriptorSetLayoutModel(nullptr),
    m_pDescriptorSetLayoutMesh(nullptr),
    m_pDescriptorSetCommon(nullptr),
    m_pAniSampler(nullptr),
    m_pFramebuffer(nullptr),
    m_pRenderPass(nullptr),
    m_pPipelineLayout(nullptr),
    m_pPipeline(nullptr)
{
    CameraComponents camSub;
    PointLightComponents pointLightSub;

    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, g_TIDModel}, {R, g_TIDWorldMatrix}}, &m_Renderables, [this](Entity entity){ onMeshAdded(entity); }, [this](Entity entity){ onMeshRemoved(entity); }},
        {{{R, g_TIDViewProjectionMatrices}}, {&camSub}, &m_Camera},
        {{&pointLightSub}, &m_PointLights}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

MeshRenderer::~MeshRenderer()
{
    // Delete all model rendering resources
    for (Entity entity : m_ModelRenderResources.getIDs()) {
        onMeshRemoved(entity);
    }

    delete m_pPointLightBuffer;
    delete m_pCommandList;
    delete m_pCommandPool;
    delete m_pDescriptorSetCommon;
    delete m_pDescriptorSetLayoutCommon;
    delete m_pDescriptorSetLayoutModel;
    delete m_pDescriptorSetLayoutMesh;
    delete m_pFramebuffer;
    delete m_pRenderPass;
    delete m_pPipelineLayout;
    delete m_pPipeline;
}

bool MeshRenderer::init()
{
    m_pCommandPool = m_pDevice->createCommandPool(COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS, m_pDevice->getQueueFamilyIndices().Graphics);
    if (!m_pCommandPool) {
        return false;
    }

    m_pCommandPool->allocateCommandLists(&m_pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY);
    if (!m_pCommandList) {
        return false;
    }

    m_pModelLoader      = reinterpret_cast<ModelLoader*>(getComponentHandler(TID(ModelLoader)));
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pVPHandler        = reinterpret_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));
    m_pLightHandler     = reinterpret_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));

    if (!m_pModelLoader || !m_pTransformHandler || !m_pVPHandler || !m_pLightHandler) {
        return false;
    }

    // Retrieve anisotropic sampler
    ShaderResourceHandler* pShaderResourceHandler = reinterpret_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));

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

    if (!createRenderPass()) {
        return false;
    }

    if (!createFramebuffer()) {
        return false;
    }

    return createPipeline();
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
    m_pDevice->map(m_pPointLightBuffer, &pMappedMemory);
    memcpy(pMappedMemory, &perFrame, sizeof(PerFrameBuffer));
    m_pDevice->unmap(m_pPointLightBuffer);

    for (Entity renderableID : m_Renderables.getIDs()) {
        ModelRenderResources& modelRenderResources = m_ModelRenderResources.indexID(renderableID);

        // Prepare camera's view*proj matrix
        const ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(m_Camera[0]);

        DirectX::XMMATRIX camVP = DirectX::XMLoadFloat4x4(&vpMatrices.View) * DirectX::XMLoadFloat4x4(&vpMatrices.Projection);

        // Update per-object matrices uniform buffer
        PerObjectMatrices matrices;
        matrices.World = m_pTransformHandler->getWorldMatrix(renderableID).worldMatrix;
        DirectX::XMStoreFloat4x4(&matrices.WVP, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices.World) * camVP));

        pMappedMemory = nullptr;
        m_pDevice->map(modelRenderResources.pWVPBuffer, &pMappedMemory);
        memcpy(pMappedMemory, &matrices, sizeof(PerObjectMatrices));
        m_pDevice->unmap(modelRenderResources.pWVPBuffer);
    }
}

void MeshRenderer::recordCommands()
{
    CommandListBeginInfo beginInfo = {};
    beginInfo.pRenderPass   = m_pRenderPass;
    beginInfo.Subpass       = 0u;
    beginInfo.pFramebuffer  = m_pFramebuffer;
    m_pCommandList->begin(COMMAND_LIST_USAGE::WITHIN_RENDER_PASS, &beginInfo);

    RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.pFramebuffer        = m_pFramebuffer;
    renderPassBeginInfo.ClearColors         = { {0.0f, 0.0f, 0.0f, 0.0f} };
    renderPassBeginInfo.ClearDepthStencilValue.Depth = 1.0f;

    m_pCommandList->beginRenderPass(m_pRenderPass, renderPassBeginInfo);

    if (m_Renderables.size() == 0 || m_Camera.size() == 0) {
        m_pCommandList->end();
        return;
    }

    m_pCommandList->bindPipeline(m_pPipeline);

    m_pCommandList->bindDescriptorSet(m_pDescriptorSetCommon);

    for (Entity renderableID : m_Renderables.getIDs()) {
        Model* pModel = m_pModelLoader->getModel(renderableID);
        const ModelRenderResources& modelRenderResources = m_ModelRenderResources.indexID(renderableID);

        m_pCommandList->bindDescriptorSet(modelRenderResources.pDescriptorSet);

        size_t meshIdx = 0;

        for (const Mesh& mesh : pModel->Meshes) {
            if (pModel->Materials[mesh.materialIndex].textures.empty()) {
                // Will not render the mesh if it does not have a texture
                continue;
            }

            const MeshRenderResources& meshRenderResources = modelRenderResources.MeshRenderResources[meshIdx];

            m_pCommandList->bindVertexBuffer(0, mesh.pVertexBuffer);
            m_pCommandList->bindIndexBuffer(mesh.pIndexBuffer);

            m_pCommandList->bindDescriptorSet(meshRenderResources.pDescriptorSet);

            m_pCommandList->drawIndexed(mesh.indexCount);

            meshIdx += 1;
        }
    }

    m_pCommandList->end();
}

void MeshRenderer::executeCommands()
{
    SemaphoreSubmitInfo semaphoreInfo = {};
    m_pDevice->graphicsQueueSubmit(m_pCommandList, nullptr, semaphoreInfo);
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
    m_pDescriptorSetLayoutMesh->addBindingSampledTexture(SHADER_BINDING::TEXTURE_ONE, SHADER_TYPE::FRAGMENT_SHADER);

    return m_pDescriptorSetLayoutMesh->finalize(m_pDevice);
}

bool MeshRenderer::createCommonDescriptorSet()
{
    m_pDescriptorSetCommon = m_pDevice->allocateDescriptorSet(m_pDescriptorSetLayoutCommon);
    if (!m_pDescriptorSetCommon) {
        return false;
    }

    m_pDescriptorSetCommon->updateUniformBufferDescriptor(SHADER_BINDING::PER_FRAME, m_pPointLightBuffer);
    m_pDescriptorSetCommon->updateSamplerDescriptor(SHADER_BINDING::SAMPLER_ONE, m_pAniSampler);
    return true;
}

bool MeshRenderer::createRenderPass()
{
    RenderPassInfo renderPassInfo   = {};

    // Render pass attachments
    Texture* pBackbuffer = m_pDevice->getBackBuffer();
    AttachmentInfo backBufferAttachment   = {};
    backBufferAttachment.Format           = pBackbuffer->getFormat();
    backBufferAttachment.Samples          = 1u;
    backBufferAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    backBufferAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    backBufferAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    backBufferAttachment.FinalLayout      = TEXTURE_LAYOUT::RENDER_TARGET;

    Texture* pDepthStencil = m_pDevice->getDepthStencil();
    AttachmentInfo depthStencilAttachment = {};
    depthStencilAttachment.Format           = pDepthStencil->getFormat();
    depthStencilAttachment.Samples          = 1u;
    depthStencilAttachment.LoadOp           = ATTACHMENT_LOAD_OP::CLEAR;
    depthStencilAttachment.StoreOp          = ATTACHMENT_STORE_OP::STORE;
    depthStencilAttachment.InitialLayout    = TEXTURE_LAYOUT::UNDEFINED;
    depthStencilAttachment.FinalLayout      = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;

    // Subpass
    SubpassInfo subpass = {};
    AttachmentReference backbufferRef   = {};
    backbufferRef.AttachmentIndex       = 0;
    backbufferRef.Layout                = TEXTURE_LAYOUT::RENDER_TARGET;

    AttachmentReference depthStencilRef = {};
    depthStencilRef.AttachmentIndex     = 1;
    depthStencilRef.Layout              = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;

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

bool MeshRenderer::createFramebuffer()
{
    FramebufferInfo framebufferInfo = {};
    framebufferInfo.Attachments = { m_pDevice->getBackBuffer(), m_pDevice->getDepthStencil() };
    framebufferInfo.Dimensions  = m_pDevice->getBackBuffer()->getDimensions();
    framebufferInfo.pRenderPass = m_pRenderPass;

    m_pFramebuffer = m_pDevice->createFramebuffer(framebufferInfo);
    return m_pFramebuffer;
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

    const glm::uvec2& backbufferDims = m_pDevice->getBackBuffer()->getDimensions();

    Viewport viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = (float)backbufferDims.x;
    viewport.Height   = (float)backbufferDims.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pipelineInfo.Viewports = { viewport };

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
    for (float& blendConstant : pipelineInfo.BlendStateInfo.pBlendConstants) {
        blendConstant = 1.0f;
    }

    pipelineInfo.DynamicStates  = { PIPELINE_DYNAMIC_STATE::SCISSOR };
    pipelineInfo.pLayout        = m_pPipelineLayout;
    pipelineInfo.pRenderPass    = m_pRenderPass;
    pipelineInfo.Subpass        = 0u;

    m_pPipeline = m_pDevice->createPipeline(pipelineInfo);
    return m_pPipeline;
}

void MeshRenderer::onMeshAdded(Entity entity)
{
    Model* pModel = m_pModelLoader->getModel(entity);

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
    modelRenderResources.pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::PER_OBJECT, modelRenderResources.pWVPBuffer);

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
        meshRenderResources.pDescriptorSet->updateUniformBufferDescriptor(SHADER_BINDING::MATERIAL_CONSTANTS, meshRenderResources.pMaterialBuffer);
        meshRenderResources.pDescriptorSet->updateSampledTextureDescriptor(SHADER_BINDING::TEXTURE_ONE, material.textures[0].get());

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

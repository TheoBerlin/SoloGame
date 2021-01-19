#pragma once

#define NOMINMAX
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>

#include <DirectXMath.h>

#define MAX_POINTLIGHTS 7u

class ModelLoader;
class Texture;
class TransformHandler;
class VPHandler;

struct MeshRenderResources {
    // Points at the mesh's material attributes buffer and diffuse texture
    DescriptorSet* pDescriptorSet;
    IBuffer* pMaterialBuffer;
};

struct ModelRenderResources {
    // Points at the WVP buffer
    DescriptorSet* pDescriptorSet;
    IBuffer* pWVPBuffer;

    std::vector<MeshRenderResources> MeshRenderResources;
};

class MeshRenderer : public Renderer
{
public:
    MeshRenderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler);
    ~MeshRenderer();

    bool init() override final;

    void updateBuffers() override final;
    void recordCommands() override final;
    void executeCommands(ICommandList* pPrimaryCommandList) override final;

    inline IRenderPass* getRenderPass()                     { return m_pRenderPass; }
    inline Framebuffer* getFramebuffer(uint32_t frameIndex) { return m_ppFramebuffers[frameIndex]; }

private:
    struct PointLightBuffer {
        DirectX::XMFLOAT3 Position;
        float Padding;
        DirectX::XMFLOAT3 Light;
        float RadiusReciprocal;
    };

    struct PerObjectMatrices {
        DirectX::XMFLOAT4X4 WVP, World;
    };

    struct PerFrameBuffer {
        PointLightBuffer PointLights[MAX_POINTLIGHTS];
        alignas(16) DirectX::XMFLOAT3 CameraPosition;
        uint32_t NumLights;
    };

private:
    bool createBuffers();
    bool createDescriptorSetLayouts();
    bool createCommonDescriptorSet();
    bool createRenderPass();
    bool createFramebuffers();
    bool createPipeline();

    void onMeshAdded(Entity entity);
    void onMeshRemoved(Entity entity);

private:
    IDVector m_Renderables;
    IDVector m_Camera;
    IDVector m_PointLights;

    IDDVector<ModelRenderResources> m_ModelRenderResources;

    Device* m_pDevice;
    ICommandPool* m_ppCommandPools[MAX_FRAMES_IN_FLIGHT];
    ICommandList* m_ppCommandLists[MAX_FRAMES_IN_FLIGHT];
    // Amount of command lists to reset and re-record
    uint32_t m_CommandListsToReset;

    ModelLoader* m_pModelLoader;
    TransformHandler* m_pTransformHandler;
    VPHandler* m_pVPHandler;
    LightHandler* m_pLightHandler;

    IDescriptorSetLayout* m_pDescriptorSetLayoutCommon; // Common for all models and mesh: Sampler and point lights
    IDescriptorSetLayout* m_pDescriptorSetLayoutModel;  // Per model: WVP matrices
    IDescriptorSetLayout* m_pDescriptorSetLayoutMesh;   // Per mesh: Material attributes and diffuse texture

    DescriptorSet* m_pDescriptorSetCommon;

    // Contains point lights, camera position and number of lights
    IBuffer* m_pPointLightBuffer;

    ISampler* m_pAniSampler;

    Framebuffer* m_ppFramebuffers[MAX_FRAMES_IN_FLIGHT];
    IRenderPass* m_pRenderPass;
    IPipeline* m_pPipeline;
    IPipelineLayout* m_pPipelineLayout;
};

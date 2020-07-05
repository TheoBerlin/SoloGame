#pragma once

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Utils/IDVector.hpp>

class Texture;
class UIHandler;

struct PanelRenderResources {
    DescriptorSet* pDescriptorSet;
    IBuffer* pBuffer;
};

class UIRenderer : public Renderer
{
public:
    UIRenderer(ECSCore* pECS, Device* pDevice);
    ~UIRenderer();

    bool init() override final;

    void updateBuffers() override final;
    void recordCommands() override final;
    void executeCommands() override final;

private:
    bool createDescriptorSetLayouts();
    bool createRenderPass();
    bool createFramebuffers();
    bool createPipeline();

    void onPanelAdded(Entity entity);
    void onPanelRemoved(Entity entity);

private:
    IDVector m_Panels;
    IDDVector<PanelRenderResources> m_PanelRenderResources;

    ICommandPool* m_pCommandPool;
    ICommandList* m_pCommandList;

    UIHandler* m_pUIHandler;

    // Vertex buffer
    IBuffer* m_pQuad;

    ISampler* m_pAniSampler;
    IRenderPass* m_pRenderPass;
    IFramebuffer* m_pFramebuffers[MAX_FRAMES_IN_FLIGHT];

    IDescriptorSetLayout* m_pDescriptorSetLayout;

    Viewport m_Viewport;

    IPipelineLayout* m_pPipelineLayout;
    IPipeline* m_pPipeline;
};

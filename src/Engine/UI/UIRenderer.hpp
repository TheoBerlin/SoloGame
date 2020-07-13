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
    UIRenderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler);
    ~UIRenderer();

    bool init() override final;

    void updateBuffers() override final;
    void recordCommands() override final;
    void executeCommands(ICommandList* pPrimaryCommandList) override final;

private:
    bool createDescriptorSetLayouts();
    bool createPipeline();

    void onPanelAdded(Entity entity);
    void onPanelRemoved(Entity entity);

private:
    IDVector m_Panels;
    IDDVector<PanelRenderResources> m_PanelRenderResources;

    ICommandPool* m_ppCommandPools[MAX_FRAMES_IN_FLIGHT];
    ICommandList* m_ppCommandLists[MAX_FRAMES_IN_FLIGHT];

    UIHandler* m_pUIHandler;

    // Vertex buffer
    IBuffer* m_pQuad;

    ISampler* m_pAniSampler;

    IDescriptorSetLayout* m_pDescriptorSetLayout;

    Viewport m_Viewport;

    IPipelineLayout* m_pPipelineLayout;
    IPipeline* m_pPipeline;
};

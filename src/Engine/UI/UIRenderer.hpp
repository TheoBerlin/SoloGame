#pragma once

#include <Engine/Rendering/Renderer.hpp>
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

    inline IRenderPass* getRenderPass()                     { return m_pRenderPass; }
    inline Framebuffer* getFramebuffer(uint32_t frameIndex) { return m_ppFramebuffers[frameIndex]; }

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

    UIHandler* m_pUIHandler;

    ICommandPool* m_ppCommandPools[MAX_FRAMES_IN_FLIGHT];
    ICommandList* m_ppCommandLists[MAX_FRAMES_IN_FLIGHT];
    // Amount of command lists to reset and re-record
    uint32_t m_CommandListsToReset;

    IBuffer* m_pQuad;
    ISampler* m_pAniSampler;

    IRenderPass* m_pRenderPass;
    Framebuffer* m_ppFramebuffers[MAX_FRAMES_IN_FLIGHT];

    IDescriptorSetLayout* m_pDescriptorSetLayout;

    Viewport m_Viewport;

    IPipelineLayout* m_pPipelineLayout;
    IPipeline* m_pPipeline;
};

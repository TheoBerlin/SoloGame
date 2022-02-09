#pragma once

#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Utils/IDVector.hpp>

class UIHandler;

struct PanelRenderResources {
    DescriptorSet* pDescriptorSet;
    IBuffer* pBuffer;
};

class UIRenderer : public Renderer
{
public:
    UIRenderer(Device* pDevice, RenderingHandler* pRenderingHandler);
    ~UIRenderer();

    bool Init() override final;

    void UpdateBuffers() override final;
    void RecordCommands() override final;
    void ExecuteCommands(ICommandList* pPrimaryCommandList) override final;

    inline IRenderPass* GetRenderPass()                     { return m_pRenderPass; }
    inline Framebuffer* GetFramebuffer(uint32_t frameIndex) { return m_ppFramebuffers[frameIndex]; }

private:
    bool CreateDescriptorSetLayouts();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreatePipeline();

    void OnPanelAdded(Entity entity);
    void OnPanelRemoved(Entity entity);

private:
    IDVector m_Panels;
    IDDVector<PanelRenderResources> m_PanelRenderResources;

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

#pragma once

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Utils/IDVector.hpp>

class DescriptorSet;
class Device;
class IBuffer;
class ICommandList;
class IDescriptorSetLayout;
class IFramebuffer;
class IPipeline;
class IPipelineLayout;
class IRasterizerState;
class IRenderPass;
class ISampler;
class ShaderHandler;
class Texture;
class UIHandler;
class Window;
struct Program;

struct PanelRenderResources {
    DescriptorSet* pDescriptorSet;
    IBuffer* pBuffer;
};

class UIRenderer : public Renderer
{
public:
    UIRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow);
    ~UIRenderer();

    bool init() override final;

    void updateBuffers() override final;
    void recordCommands() override final;
    void executeCommands() override final;

private:
    bool createDescriptorSetLayouts();
    bool createCommonDescriptorSet();
    bool createRenderPass();
    bool createFramebuffer();
    bool createPipeline();

    void onPanelAdded(Entity entity);
    void onPanelRemoved(Entity entity);

private:
    IDVector m_Panels;
    IDDVector<PanelRenderResources> m_PanelRenderResources;

    ICommandList* m_pCommandList;

    UIHandler* m_pUIHandler;

    // Vertex buffer
    IBuffer* m_pQuad;

    Texture* m_pRenderTarget;
    Texture* m_pDepthStencil;
    ISampler* m_pAniSampler;
    IRenderPass* m_pRenderPass;
    IFramebuffer* m_pFramebuffer;

    IDescriptorSetLayout* m_pDescriptorSetLayoutCommon; // Common for all panels: Sampler
    IDescriptorSetLayout* m_pDescriptorSetLayoutPanel;  // Per panel: Buffer containing transform and highlight settings, texture
    DescriptorSet* m_pDescriptorSetCommon;

    Viewport m_Viewport;

    IPipelineLayout* m_pPipelineLayout;
    IPipeline* m_pPipeline;
};

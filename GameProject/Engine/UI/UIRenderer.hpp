#pragma once

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Utils/IDVector.hpp>

class DescriptorSet;
class Device;
class IBuffer;
class ICommandList;
class IDescriptorSetLayout;
class IRasterizerState;
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
    void onPanelAdded(Entity entity);
    void onPanelRemoved(Entity entity);

private:
    IDVector m_Panels;
    IDDVector<PanelRenderResources> m_PanelRenderResources;

    ICommandList* m_pCommandList;

    UIHandler* m_pUIHandler;

    ShaderHandler* m_pShaderHandler;

    Program* m_pUIProgram;

    // Vertex buffer
    IBuffer* m_pQuad;

    Texture* m_pRenderTarget;
    Texture* m_pDepthStencil;
    ISampler* m_pAniSampler;
    IRasterizerState* m_pRasterizerState;

    IDescriptorSetLayout* m_pDescriptorSetLayoutCommon; // Common for all panels: Sampler
    IDescriptorSetLayout* m_pDescriptorSetLayoutPanel;  // Per panel: Buffer containing transform and highlight settings, texture
    DescriptorSet* m_pDescriptorSetCommon;

    Viewport m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};

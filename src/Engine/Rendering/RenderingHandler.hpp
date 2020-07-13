#pragma once

#include <Engine/Rendering/MeshRenderer.hpp>
#include <Engine/UI/UIRenderer.hpp>

class RenderingHandler
{
public:
    RenderingHandler(ECSCore* pECS, Device* pDevice);
    ~RenderingHandler();

    bool init();

    void render();

    inline ICommandList* getCurrentPrimaryCommandList()     { return m_ppCommandLists[m_pDevice->getFrameIndex()]; }
    inline IRenderPass* getRenderPass()                     { return m_pRenderPass; }
    inline Framebuffer* getCurrentFramebufferBackDepth()    { return m_ppFramebuffersBackDepth[m_pDevice->getFrameIndex()]; }

private:
    bool createRenderPass();
    bool createFramebuffers();

    void beginFrame();
    void endFrame();
    void updateBuffers();
    void recordCommandBuffers();
    void executeCommandBuffers();

private:
    ECSCore* m_pECS;
    Device* m_pDevice;

    std::vector<Renderer*> m_Renderers;

    MeshRenderer m_MeshRenderer;
    UIRenderer m_UIRenderer;

    // Primary command lists
    ICommandPool* m_ppCommandPools[MAX_FRAMES_IN_FLIGHT];
    ICommandList* m_ppCommandLists[MAX_FRAMES_IN_FLIGHT];

    IRenderPass* m_pRenderPass;
    Framebuffer* m_ppFramebuffersBackDepth[MAX_FRAMES_IN_FLIGHT];

    ISemaphore* m_ppRenderingSemaphores[MAX_FRAMES_IN_FLIGHT];
    IFence* m_pPrimaryBufferFence;
};

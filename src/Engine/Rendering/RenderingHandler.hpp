#pragma once

#include <Engine/Rendering/MeshRenderer.hpp>
#include <Engine/UI/UIRenderer.hpp>

class RenderingCore;

class RenderingHandler
{
public:
    RenderingHandler(RenderingCore* pRenderingCore);
    ~RenderingHandler();

    bool Init();

    void render();

    // waitAllFrames blocks the calling thread until all currently queued command lists have finished executing
    void waitAllFrames();

    inline ICommandList* getCurrentPrimaryCommandList()     { return m_ppCommandLists[m_pDevice->getFrameIndex()]; }
    inline IFence** getFences()                             { return m_ppPrimaryBufferFences; }

private:
    void beginFrame();
    void endFrame();
    void updateBuffers();
    void recordSecondaryCommandBuffers();
    void recordPrimaryCommandBuffer();

private:
    Device* m_pDevice;

    std::vector<Renderer*> m_Renderers;

    MeshRenderer* m_pMeshRenderer;
    UIRenderer* m_pUIRenderer;

    // Primary command lists
    ICommandPool* m_ppCommandPools[MAX_FRAMES_IN_FLIGHT];
    ICommandList* m_ppCommandLists[MAX_FRAMES_IN_FLIGHT];

    ISemaphore* m_ppRenderingSemaphores[MAX_FRAMES_IN_FLIGHT];
    IFence* m_ppPrimaryBufferFences[MAX_FRAMES_IN_FLIGHT];
};

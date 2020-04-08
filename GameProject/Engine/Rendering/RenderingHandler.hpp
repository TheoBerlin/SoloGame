#pragma once

#include <Engine/Rendering/MeshRenderer.hpp>
#include <Engine/UI/UIRenderer.hpp>

class RenderingHandler
{
public:
    RenderingHandler(ECSCore* pECS, Display* pDisplay);
    ~RenderingHandler();

    bool init();

    void render();

private:
    void recordCommandBuffers();
    void executeCommandBuffers();

private:
    ECSCore* m_pECS;
    Display* m_pDisplay;

    std::vector<Renderer*> m_Renderers;

    MeshRenderer m_MeshRenderer;
    UIRenderer m_UIRenderer;
};

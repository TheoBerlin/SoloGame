#pragma once

#include <Engine/UI/Panel.hpp>
#include <Engine/UI/ButtonSystem.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>

class RenderingCore;

class UICore
{
public:
    UICore(RenderingCore* pRenderingCore);
    ~UICore() = default;

    UIHandler* GetPanelHandler()    { return &m_PanelHandler; }
    TextRenderer* GetTextRenderer() { return &m_TextRenderer; }

    bool Init();

private:
    // Component Handlers
    UIHandler m_PanelHandler;
    TextRenderer m_TextRenderer;

    // Systems
    ButtonSystem m_ButtonSystem;
};

#pragma once

#include <Engine/UI/Panel.hpp>
#include <Engine/UI/ButtonSystem.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>

class Device;
class Window;

class UICore
{
public:
    UICore(ECSCore* pECS, Device* pDevice, Window* pWindow);
    ~UICore();

    UIHandler& getPanelHandler()    { return m_PanelHandler; }

private:
    // Component Handlers
    UIHandler m_PanelHandler;
    TextRenderer m_TextRenderer;

    // Systems
    ButtonSystem m_ButtonSystem;
};

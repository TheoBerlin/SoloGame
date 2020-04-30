#pragma once

#include <Engine/UI/Panel.hpp>
#include <Engine/UI/ButtonSystem.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>

class IDevice;
class Window;

class UICore
{
public:
    UICore(ECSCore* pECS, IDevice* pDevice, Window* pWindow);
    ~UICore();

    ButtonSystem& getButtonSystem() { return m_ButtonSystem; }
    UIHandler& getPanelHandler()    { return m_PanelHandler; }

private:
    // Component Handlers
    UIHandler m_PanelHandler;
    TextRenderer m_TextRenderer;

    // Systems
    ButtonSystem m_ButtonSystem;
};

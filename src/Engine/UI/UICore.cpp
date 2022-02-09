#include "UICore.hpp"

#include <Engine/Rendering/Window.hpp>

UICore::UICore(RenderingCore* pRenderingCore)
    :   m_PanelHandler(pRenderingCore->GetDevice())
    ,   m_TextRenderer(pRenderingCore->GetDevice())
    ,   m_ButtonSystem(pRenderingCore->GetWindow())
{}

bool UICore::Init()
{
    return m_TextRenderer.Init() && m_PanelHandler.Init();
}

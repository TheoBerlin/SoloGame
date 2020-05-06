#include "UICore.hpp"

#include <Engine/Rendering/Window.hpp>

UICore::UICore(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :m_PanelHandler(pECS, pDevice, pWindow),
    m_TextRenderer(pECS, pDevice),
    m_ButtonSystem(pECS, pWindow)
{}

UICore::~UICore()
{}

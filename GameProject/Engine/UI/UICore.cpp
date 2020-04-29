#include "UICore.hpp"

#include <Engine/Rendering/Display.hpp>

UICore::UICore(ECSCore* pECS, Display* pDisplay)
    :m_PanelHandler(pECS, pDisplay),
    m_TextRenderer(pECS, pDisplay->getDevice(), pDisplay->getDeviceContext()),
    m_ButtonSystem(pECS, pDisplay->getClientWidth(), pDisplay->getClientHeight())
{}

UICore::~UICore()
{}

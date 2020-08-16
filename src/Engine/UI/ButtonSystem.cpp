#include "ButtonSystem.hpp"

#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ButtonSystem::ButtonSystem(ECSCore* pECS, Window* pWindow)
    :System(pECS),
    m_pUIHandler(nullptr),
    m_ClientWidth(pWindow->getWidth()),
    m_ClientHeight(pWindow->getHeight()),
    m_pInputHandler(pWindow->getInputHandler()),
    m_PressedButtonExists(false),
    m_PressedButton(UINT64_MAX),
    m_HoveredButtonExists(false),
    m_HoveredButton(UINT64_MAX)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{RW, tid_UIPanel}, {R, tid_UIButton}}, &m_Buttons}
    };
    sysReg.pSystem = this;

    enqueueRegistration(sysReg);
}

bool ButtonSystem::initSystem()
{
    m_pUIHandler = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    return m_pUIHandler;
}

void ButtonSystem::update(float dt)
{
    if (m_pInputHandler->cursorIsHidden()) {
        return;
    }

    const glm::dvec2& mousePosition = m_pInputHandler->getMousePosition();

    for (Entity entity : m_Buttons.getIDs()) {
        UIPanel& panel = m_pUIHandler->panels.indexID(entity);
        UIButton& button = m_pUIHandler->buttons.indexID(entity);
        panel.highlight = button.defaultHighlight;

		unsigned int mouseX = (unsigned int)mousePosition.x;
		unsigned int mouseY = (unsigned int)(m_ClientHeight - mousePosition.y);

        // Check that the mouse and the button align horizontally
        unsigned int buttonXLeft = (unsigned int)(panel.position.x * m_ClientWidth);
        if (mouseX < buttonXLeft) {
            continue;
        }

        unsigned int buttonXRight = (unsigned int)(buttonXLeft + panel.size.x * m_ClientWidth);
        if (mouseX > buttonXRight) {
            continue;
        }

        // Check that the mouse and the button align vertically
        unsigned int buttonYDown = (unsigned int)(panel.position.y * m_ClientHeight);
        if (mouseY < buttonYDown) {
            continue;
        }

        unsigned int buttonYUp = (unsigned int)(buttonYDown + panel.size.y * m_ClientHeight);
        if (mouseY > buttonYUp) {
            continue;
        }

        // The mouse is hovering the button. Now there are three possible states to check and handle:
        // 1. The button is being pressed: enable pressed highlight
        // 2. The mouse is already pressed and is not being pressed now: enable default highlight and trigger button function
        // 3. The mouse is not already pressed and is not being pressed now: enable hover highlight
		m_HoveredButtonExists = true;
		m_HoveredButton = entity;

        if (m_pInputHandler->mouseButtonState(GLFW_MOUSE_BUTTON_LEFT)) {
            // State 1
            panel.highlight = button.pressHighlight;
            m_PressedButtonExists = true;
            m_PressedButton = entity;
        } else {
            if (m_PressedButtonExists && m_PressedButton == entity) {
                // State 2
                m_PressedButtonExists = false;

                panel.highlight = button.defaultHighlight;
                button.onPress();
            } else {
                // State 3
                panel.highlight = button.hoverHighlight;
            }
        }
    }
}

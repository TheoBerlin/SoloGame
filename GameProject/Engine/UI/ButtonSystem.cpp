#include "ButtonSystem.hpp"

#include <Engine/Rendering/Display.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ButtonSystem::ButtonSystem(ECSCore* pECS, unsigned int clientWidth, unsigned int clientHeight)
    :System(pECS),
    m_ClientWidth(clientWidth),
    m_ClientHeight(clientHeight),
    m_PressedButtonExists(false),
    m_PressedButton(0)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{RW, tid_UIPanel}, {R, tid_UIButton}}, &m_Buttons}
    };
    sysReg.pSystem = this;

    subscribeToComponents(sysReg);
}

ButtonSystem::~ButtonSystem()
{}

bool ButtonSystem::initSystem()
{
    m_pUIhandler = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    InputHandler* pInputHandler = static_cast<InputHandler*>(getComponentHandler(TID(InputHandler)));

    m_pMouseState = pInputHandler->getMouseState();

    return m_pUIhandler && pInputHandler;
}

void ButtonSystem::update(float dt)
{
    if (m_pMouseState->positionMode == DirectX::Mouse::Mode::MODE_RELATIVE) {
        return;
    }

    for (Entity entity : m_Buttons.getIDs()) {
        UIPanel& panel = m_pUIhandler->panels.indexID(entity);
        UIButton& button = m_pUIhandler->buttons.indexID(entity);
        panel.highlight = button.defaultHighlight;

		unsigned int mouseX = (unsigned int)m_pMouseState->x;
		unsigned int mouseY = (unsigned int)(m_ClientHeight - m_pMouseState->y);

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

        if (m_pMouseState->leftButton) {
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

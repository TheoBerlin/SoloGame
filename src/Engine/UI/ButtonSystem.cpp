#include "ButtonSystem.hpp"

#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ButtonSystem::ButtonSystem(Window* pWindow)
    :m_pUIHandler(nullptr),
    m_ClientWidth(pWindow->getWidth()),
    m_ClientHeight(pWindow->getHeight()),
    m_pInputHandler(pWindow->GetInputHandler()),
    m_PressedButtonExists(false),
    m_PressedButton(UINT32_MAX),
    m_HoveredButtonExists(false),
    m_HoveredButton(UINT32_MAX)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {
            .pSubscriber = &m_Buttons,
            .ComponentAccesses =
            {
                { RW, UIPanelComponent::Type() }, {R, UIButtonComponent::Type() }
            },
        }
    };

    RegisterSystem(TYPE_NAME(ButtonSystem), sysReg);
}

void ButtonSystem::Update(float dt)
{
    UNREFERENCED_VARIABLE(dt);

    if (m_pInputHandler->cursorIsHidden()) {
        return;
    }

    ECSCore* pECS = ECSCore::GetInstance();
    ComponentArray<UIPanelComponent>* pPanelComponents = pECS->GetComponentArray<UIPanelComponent>();
    ComponentArray<UIButtonComponent>* pButtonComponents = pECS->GetComponentArray<UIButtonComponent>();

    const glm::dvec2& mousePosition = m_pInputHandler->getMousePosition();

    for (Entity entity : m_Buttons.GetIDs()) {
        UIPanelComponent& panel = pPanelComponents->GetData(entity);
        UIButtonComponent& button = pButtonComponents->GetData(entity);
        panel.highlight = button.defaultHighlight;

		const unsigned int mouseX = (unsigned int)mousePosition.x;
		const unsigned int mouseY = (unsigned int)(m_ClientHeight - mousePosition.y);

        // Check that the mouse and the button align horizontally
        const unsigned int buttonXLeft = (unsigned int)(panel.position.x * m_ClientWidth);
        if (mouseX < buttonXLeft) {
            continue;
        }

        const unsigned int buttonXRight = (unsigned int)(buttonXLeft + panel.size.x * m_ClientWidth);
        if (mouseX > buttonXRight) {
            continue;
        }

        // Check that the mouse and the button align vertically
        const unsigned int buttonYDown = (unsigned int)(panel.position.y * m_ClientHeight);
        if (mouseY < buttonYDown) {
            continue;
        }

        const unsigned int buttonYUp = (unsigned int)(buttonYDown + panel.size.y * m_ClientHeight);
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

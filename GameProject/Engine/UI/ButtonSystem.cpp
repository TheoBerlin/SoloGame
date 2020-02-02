#include "ButtonSystem.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/UI/Panel.hpp>

ButtonSystem::ButtonSystem(ECSInterface* ecs, unsigned int clientWidth, unsigned int clientHeight)
    :System(ecs),
    clientWidth(clientWidth),
    clientHeight(clientHeight),
    pressedButtonExists(false),
    pressedButton(0)
{
    SystemRegistration sysReg = {
    {
        {{{RW, tid_UIPanel}, {R, tid_UIButton}}, &buttons}
    },
    this};

    this->subscribeToComponents(&sysReg);

    const std::type_index tid_UIHandler = std::type_index(typeid(UIHandler));
    this->UIhandler = static_cast<UIHandler*>(ecs->systemSubscriber.getComponentHandler(tid_UIHandler));

    const std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));
    InputHandler* inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));

    this->mouseState = inputHandler->getMouseState();
}

ButtonSystem::~ButtonSystem()
{}

void ButtonSystem::update(float dt)
{
    if (mouseState->positionMode == DirectX::Mouse::Mode::MODE_RELATIVE) {
        return;
    }

    bool isHoveringButton = false;

    for (Entity entity : buttons.getVec()) {
        UIPanel& panel = UIhandler->panels.indexID(entity);

		unsigned int mouseX = (unsigned int)mouseState->x;
		unsigned int mouseY = (unsigned int)(clientHeight - mouseState->y);

        // Check that the mouse and the button align horizontally
        unsigned int buttonXLeft = (unsigned int)(panel.position.x * clientWidth);
        if (mouseX < buttonXLeft) {
            continue;
        }

        unsigned int buttonXRight = (unsigned int)(buttonXLeft + panel.size.x * clientWidth);
        if (mouseX > buttonXRight) {
            continue;
        }

        // Check that the mouse and the button align vertically
        unsigned int buttonYDown = (unsigned int)(panel.position.y * clientHeight);
        if (mouseY < buttonYDown) {
            continue;
        }

        unsigned int buttonYUp = (unsigned int)(buttonYDown + panel.size.y * clientHeight);
        if (mouseY > buttonYUp) {
            continue;
        }

        // The mouse is hovering the button. Now there are three possible states to check and handle:
        // 1. The button is being pressed: enable pressed highlight
        // 2. The mouse is already pressed and is not being pressed now: enable default highlight and trigger button function
        // 3. The mouse is not already pressed and is not being pressed now: enable hover highlight
        UIButton& button = UIhandler->buttons.indexID(entity);
        isHoveringButton = true;

		hoveredButtonExists = true;
		hoveredButton = entity;

        if (mouseState->leftButton) {
            // State 1
            panel.highlight = button.pressHighlight;
            pressedButtonExists = true;
            pressedButton = entity;
        } else {
            if (pressedButtonExists && pressedButton == entity) {
                // State 2
                pressedButtonExists = false;

                panel.highlight = button.defaultHighlight;
                button.onPress();
            } else {
                // State 3
                panel.highlight = button.hoverHighlight;
            }
        }
    }

    if (!isHoveringButton) {
        if (pressedButtonExists) {
            if (!mouseState->leftButton) {
                // The previously pressed button is not being hovered anymore and left mouse is not being held
                pressedButtonExists = false;
                UIhandler->panels.indexID(pressedButton).highlight = UIhandler->buttons.indexID(pressedButton).defaultHighlight;
            } else {
                // The previously pressed button is not being hovered, but the left mouse button is still being held
                UIhandler->panels.indexID(pressedButton).highlight = UIhandler->buttons.indexID(pressedButton).hoverHighlight;
            }
        }

        if (hoveredButtonExists && (!pressedButtonExists || (hoveredButton != pressedButton))) {
            // The previously (non-pressed) hovered button is not being hovered anymore
            UIhandler->panels.indexID(hoveredButton).highlight = UIhandler->buttons.indexID(hoveredButton).defaultHighlight;
        }

		hoveredButtonExists = false;
    }
}

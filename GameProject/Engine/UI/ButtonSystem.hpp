#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/InputHandler.hpp>

class UIHandler;

class ButtonSystem : public System
{
public:
    ButtonSystem(ECSInterface* ecs, unsigned int windowWidth, unsigned int windowHeight);
    ~ButtonSystem();

    void update(float dt);

private:
    IDVector<Entity> buttons;

    UIHandler* UIhandler;

    // Used for translating panel positions from [0,1] to [0, windowWidth or windowHeight]
    unsigned int clientWidth, clientHeight;

    DirectX::Mouse::State* mouseState;

    // The bools are needed because the entity ID can't be set to -1 (it's unsigned).
    // Here, the assumption is made that only one button can be hovered or pressed at a time.
    bool pressedButtonExists;
    Entity pressedButton;

    bool hoveredButtonExists;
    Entity hoveredButton;
};

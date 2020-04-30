#pragma once

#define NOMINMAX
#include <Engine/ECS/System.hpp>
#include <Engine/InputHandler.hpp>

class InputHandler;
class UIHandler;
class Window;

class ButtonSystem : public System
{
public:
    ButtonSystem(ECSCore* pECS, Window* pWindow);
    ~ButtonSystem();

    virtual bool initSystem() override;

    void update(float dt);

private:
    IDVector m_Buttons;

    UIHandler* m_pUIhandler;

    // Used for translating panel positions from [0,1] to [0, windowWidth or windowHeight]
    unsigned int m_ClientWidth, m_ClientHeight;

    InputHandler* m_pInputHandler;

    // The bools are needed because the entity ID can't be set to -1 (it's unsigned).
    // Here, the assumption is made that only one button can be hovered or pressed at a time.
    bool m_PressedButtonExists;
    Entity m_PressedButton;

    bool m_HoveredButtonExists;
    Entity m_HoveredButton;
};

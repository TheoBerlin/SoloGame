#pragma once

#define NOMINMAX
#include <Windows.h>
#include <Engine/ECS/ComponentHandler.hpp>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

class InputHandler : public ComponentHandler
{
public:
    InputHandler(ECSCore* pECS, HWND window);
    ~InputHandler();

    virtual bool init() override;

    // Updates the states of the keyboard and mouse
    void update();

    DirectX::Keyboard::State* getKeyboardState();
    DirectX::Mouse::State* getMouseState();

    void setMouseMode(DirectX::Mouse::Mode mode);
    void setMouseVisibility(bool visible);

private:
    DirectX::Keyboard m_Keyboard;
    DirectX::Keyboard::State m_KeyboardState;

    DirectX::Mouse m_Mouse;
    DirectX::Mouse::State m_MouseState;
    DirectX::Mouse::ButtonStateTracker m_MouseBtnTracker;

    HWND m_Window;
};

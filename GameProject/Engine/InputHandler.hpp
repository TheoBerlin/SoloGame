#pragma once

#define NOMINMAX
#include <Windows.h>
#include <Engine/ECS/ComponentHandler.hpp>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

class InputHandler : public ComponentHandler
{
public:
    InputHandler(SystemSubscriber* sysSubscriber, HWND window);
    ~InputHandler();

    // Updates the states of the keyboard and mouse
    void update();

    DirectX::Keyboard::State* getKeyboardState();
    DirectX::Mouse::State* getMouseState();

private:
    DirectX::Keyboard keyboard;
    DirectX::Keyboard::State keyboardState;

    DirectX::Mouse mouse;
    DirectX::Mouse::State mouseState;
    DirectX::Mouse::ButtonStateTracker mouseBtnTracker;
};

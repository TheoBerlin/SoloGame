#include "InputHandler.hpp"

InputHandler::InputHandler(ECSCore* pECS, HWND window)
    :ComponentHandler({}, pECS, std::type_index(typeid(InputHandler)))
{
    mouse.SetWindow(window);
    mouse.SetMode(mouse.MODE_RELATIVE);
}

InputHandler::~InputHandler()
{}

void InputHandler::update()
{
    keyboardState = keyboard.GetState();
    mouseState = mouse.GetState();
    mouseBtnTracker.Update(mouseState);
}

DirectX::Keyboard::State* InputHandler::getKeyboardState()
{
    return &keyboardState;
}

DirectX::Mouse::State* InputHandler::getMouseState()
{
    return &mouseState;
}

void InputHandler::setMouseMode(DirectX::Mouse::Mode mode)
{
    mouse.SetMode(mode);
}

void InputHandler::setMouseVisibility(bool visible)
{
    ShowCursor(visible);
    mouse.SetVisible(visible);
    ShowCursor(visible);
}

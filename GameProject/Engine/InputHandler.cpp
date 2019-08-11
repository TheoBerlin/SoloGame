#include "InputHandler.hpp"

InputHandler::InputHandler(SystemSubscriber* sysSubscriber, HWND window)
    :ComponentHandler({}, sysSubscriber, std::type_index(typeid(InputHandler)))
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
}

DirectX::Keyboard::State* InputHandler::getKeyboardState()
{
    return &keyboardState;
}

DirectX::Mouse::State* InputHandler::getMouseState()
{
    return &mouseState;
}

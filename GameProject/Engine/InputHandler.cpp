#include "InputHandler.hpp"

#include <Engine/Utils/ECSUtils.hpp>

InputHandler::InputHandler(ECSCore* pECS, HWND window)
    :ComponentHandler(pECS, TID(InputHandler)),
    m_Window(window)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    registerHandler(handlerReg);
}

InputHandler::~InputHandler()
{}

bool InputHandler::init()
{
    m_Mouse.SetWindow(m_Window);
    m_Mouse.SetMode(m_Mouse.MODE_RELATIVE);
    return true;
}

void InputHandler::update()
{
    m_KeyboardState = m_Keyboard.GetState();
    m_MouseState = m_Mouse.GetState();
    m_MouseBtnTracker.Update(m_MouseState);
}

DirectX::Keyboard::State* InputHandler::getKeyboardState()
{
    return &m_KeyboardState;
}

DirectX::Mouse::State* InputHandler::getMouseState()
{
    return &m_MouseState;
}

void InputHandler::setMouseMode(DirectX::Mouse::Mode mode)
{
    m_Mouse.SetMode(mode);
}

void InputHandler::setMouseVisibility(bool visible)
{
    ShowCursor(visible);
    m_Mouse.SetVisible(visible);
    ShowCursor(visible);
}

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

bool InputHandler::initHandler()
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

// InputHandlerV2
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/Logger.hpp>

InputHandlerV2::InputHandlerV2()
    :m_MousePosition(0.0, 0.0),
    m_MouseMove(0.0, 0.0),
    m_RawMotionEnabled(false)
{
    for (bool& keyState : m_pKeyStates) {
        keyState = false;
    }

    for (bool& buttonState : m_pMouseButtonStates) {
        buttonState = false;
    }
}

InputHandlerV2::~InputHandlerV2()
{}

void InputHandlerV2::init(GLFWwindow* pWindow)
{
    m_pWindow = pWindow;

    glfwSetKeyCallback(pWindow, InputHandlerV2::keyActionCallbackStatic);
    glfwSetMouseButtonCallback(pWindow, InputHandlerV2::mouseButtonCallbackStatic);

    glfwGetCursorPos(pWindow, &m_MousePosition.x, &m_MousePosition.y);
}

void InputHandlerV2::update()
{
    if (m_RawMotionEnabled) {
        glfwGetCursorPos(m_pWindow, &m_MouseMove.x, &m_MouseMove.y);
    } else {
        glfwGetCursorPos(m_pWindow, &m_MousePosition.x, &m_MousePosition.y);
    }
}

void InputHandlerV2::showCursor()
{
    glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    m_RawMotionEnabled = false;
}

bool InputHandlerV2::hideCursor()
{
    if (m_RawMotionEnabled) {
        return true;
    }

    if (!glfwRawMouseMotionSupported()) {
        LOG_ERROR("Raw mouse motion is not supported");
        return false;
    }

    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    m_RawMotionEnabled = true;
    return true;
}

void InputHandlerV2::keyActionCallbackStatic(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods)
{
    // Retrieve pointer to InputHandler instance
    Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
    pWindow->getInputHandler()->keyActionCallback(key, scancode, action, mods);
}

void InputHandlerV2::mouseButtonCallbackStatic(GLFWwindow* pGLFWWindow, int button, int action, int mods)
{
    // Retrieve pointer to InputHandler instance
    Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
    pWindow->getInputHandler()->mouseButtonCallback(button, action, mods);
}

void InputHandlerV2::keyActionCallback(int key, int scancode, int action, int mods)
{
    m_pKeyStates[key] = action == GLFW_PRESS;
}

void InputHandlerV2::mouseButtonCallback(int button, int action, int mods)
{
    m_pMouseButtonStates[button] = action == GLFW_PRESS;
}

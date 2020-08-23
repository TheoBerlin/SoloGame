#include "InputHandler.hpp"

#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/Logger.hpp>

InputHandler::InputHandler()
    :m_pWindow(nullptr),
    m_Enabled(true),
    m_pKeyStates(),
    m_pMouseButtonStates(),
    m_MouseCenter(0.0, 0.0),
    m_MousePosition(0.0, 0.0),
    m_MouseMove(0.0, 0.0),
    m_RawMotionEnabled(false)
{
    std::fill_n(m_pKeyStates.data(), m_pKeyStates.size(), false);
    std::fill_n(m_pMouseButtonStates.data(), m_pMouseButtonStates.size(), false);
}

void InputHandler::init(GLFWwindow* pWindow, uint32_t windowWidth, uint32_t windowHeight)
{
    m_pWindow = pWindow;

    m_MouseCenter = glm::dvec2((double)windowWidth, (double)windowHeight) / 2.0;

    glfwSetKeyCallback(pWindow, InputHandler::keyActionCallbackStatic);
    glfwSetMouseButtonCallback(pWindow, InputHandler::mouseButtonCallbackStatic);

    glfwGetCursorPos(pWindow, &m_MousePosition.x, &m_MousePosition.y);
}

void InputHandler::update()
{
    if (m_RawMotionEnabled) {
        glfwGetCursorPos(m_pWindow, &m_MouseMove.x, &m_MouseMove.y);
        m_MouseMove -= m_MouseCenter;
        glfwSetCursorPos(m_pWindow, m_MouseCenter.x, m_MouseCenter.y);
    } else {
        glfwGetCursorPos(m_pWindow, &m_MousePosition.x, &m_MousePosition.y);
    }
}

void InputHandler::disable()
{
    m_Enabled = false;
    std::fill_n(m_pKeyStates.data(), m_pKeyStates.size(), false);
    std::fill_n(m_pMouseButtonStates.data(), m_pMouseButtonStates.size(), false);
}

void InputHandler::showCursor()
{
    glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    m_RawMotionEnabled = false;
}

bool InputHandler::hideCursor()
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

    glfwSetCursorPos(m_pWindow, m_MouseCenter.x, m_MouseCenter.y);

    m_RawMotionEnabled = true;
    return true;
}

void InputHandler::keyActionCallbackStatic(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods)
{
    if (key != GLFW_KEY_UNKNOWN) {
        // Retrieve pointer to InputHandler instance
        Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
        pWindow->getInputHandler()->keyActionCallback(key, scancode, action, mods);
    }
}

void InputHandler::mouseButtonCallbackStatic(GLFWwindow* pGLFWWindow, int button, int action, int mods)
{
    // Retrieve pointer to InputHandler instance
    Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
    pWindow->getInputHandler()->mouseButtonCallback(button, action, mods);
}

void InputHandler::keyActionCallback(int key, int scancode, int action, int mods)
{
    m_pKeyStates[key] = action != GLFW_RELEASE && m_Enabled;
}

void InputHandler::mouseButtonCallback(int button, int action, int mods)
{
    m_pMouseButtonStates[button] = action != GLFW_RELEASE && m_Enabled;
}

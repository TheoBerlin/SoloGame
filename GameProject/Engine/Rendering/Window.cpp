#include "Window.hpp"

#include <Engine/Utils/Logger.hpp>

Window::Window(uint32_t clientHeight, float aspectRatio, bool windowed)
    :m_Width(uint32_t(float(clientHeight) * aspectRatio)),
    m_Height(clientHeight),
    m_pWindow(nullptr)
{}

Window::~Window()
{
    if (m_pWindow) {
        glfwDestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }

    glfwTerminate();
}

bool Window::init()
{
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    glfwSetErrorCallback(Window::glfwErrorCallback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_pWindow = glfwCreateWindow((int)m_Width, (int)m_Height, "Game Name", nullptr, nullptr);
    if (!m_pWindow) {
        LOG_ERROR("Failed to create GLFW window");
        return false;
    }

    return true;
}

void Window::pollEvents()
{
    glfwPollEvents();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(m_pWindow);
}

HWND Window::getHWND()
{
    return glfwGetWin32Window(m_pWindow);
}

void Window::glfwErrorCallback(int error, const char* pDescription)
{
    LOG_ERROR("GLFW Error [%d]: %s", error, pDescription);
}

#include "Window.hpp"

Window::Window(uint32_t clientHeight, float aspectRatio)
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

    // Callback functions eg. for key input are stateless. Setting this pointer allows them to become stateful by retrieving the Window
    // pointer inside one of the callback functions
    glfwSetWindowUserPointer(m_pWindow, this);

    m_InputHandler.init(m_pWindow, m_Width, m_Height);

    return true;
}

void Window::show()
{
    glfwShowWindow(m_pWindow);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::Close()
{
    glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
}

HWND Window::getHWND() const
{
    return glfwGetWin32Window(m_pWindow);
}

std::vector<std::string> Window::getRequiredInstanceExtensions()
{
    const char** ppExtensions = nullptr;
    uint32_t extensionCount = 0u;
    ppExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<std::string> extensionStrings((size_t)extensionCount);
    for (size_t extensionIdx = 0u; extensionIdx < extensionStrings.size(); extensionIdx += 1u) {
        extensionStrings[extensionIdx] = std::string(ppExtensions[extensionIdx]);
    }

    return extensionStrings;
}

void Window::glfwErrorCallback(int error, const char* pDescription)
{
    LOG_ERRORF("GLFW Error [%d]: %s", error, pDescription);
}

#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdint.h>

class Window
{
public:
    Window(uint32_t clientHeight, float aspectRatio, bool windowed);
    ~Window();

    bool init();

    uint32_t getWidth() const { return m_Width; }
    uint32_t getHeight() const { return m_Height; }

    void pollEvents();

    bool shouldClose();
    HWND getHWND();

private:
    static void glfwErrorCallback(int error, const char* pDescription);

private:
    uint32_t m_Width, m_Height;

    GLFWwindow* m_pWindow;
};

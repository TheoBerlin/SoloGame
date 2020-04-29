#pragma once

#include <Engine/InputHandler.hpp>

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

    void pollEvents();

    bool shouldClose();
    HWND getHWND();

    InputHandlerV2* getInputHandler() { return &m_InputHandler; }

private:
    static void glfwErrorCallback(int error, const char* pDescription);

private:
    uint32_t m_ClientWidth, m_ClientHeight;

    GLFWwindow* m_pWindow;
    InputHandlerV2 m_InputHandler;
};

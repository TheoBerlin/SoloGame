#pragma once

#include <Engine/InputHandler.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdint.h>

class Window
{
public:
    Window(uint32_t clientHeight, float aspectRatio);
    ~Window();

    bool init();
    void show();

    static void pollEvents();
    void close();
    bool shouldClose() const { return glfwWindowShouldClose(m_pWindow); }

    HWND getHWND() const;
    // Vulkan-specific
    static std::vector<std::string> getRequiredInstanceExtensions();

    uint32_t getWidth() const   { return m_Width; }
    uint32_t getHeight() const  { return m_Height; }

    InputHandler* getInputHandler() { return &m_InputHandler; }

private:
    static void glfwErrorCallback(int error, const char* pDescription);

private:
    uint32_t m_Width, m_Height;

    GLFWwindow* m_pWindow;
    InputHandler m_InputHandler;
};

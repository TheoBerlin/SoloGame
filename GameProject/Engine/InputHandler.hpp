#pragma once

#define NOMINMAX
#include <Engine/ECS/ComponentHandler.hpp>

#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

#include <Windows.h>

class InputHandler : public ComponentHandler
{
public:
    InputHandler(ECSCore* pECS, HWND window);
    ~InputHandler();

    virtual bool initHandler() override;

    // Updates the states of the keyboard and mouse
    void update();

    DirectX::Keyboard::State* getKeyboardState();
    DirectX::Mouse::State* getMouseState();

    void setMouseMode(DirectX::Mouse::Mode mode);
    void setMouseVisibility(bool visible);

private:
    DirectX::Keyboard m_Keyboard;
    DirectX::Keyboard::State m_KeyboardState;

    DirectX::Mouse m_Mouse;
    DirectX::Mouse::State m_MouseState;
    DirectX::Mouse::ButtonStateTracker m_MouseBtnTracker;

    HWND m_Window;
};

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// TODO: Replace InputHandler
class InputHandlerV2
{
public:
    InputHandlerV2();
    ~InputHandlerV2();

    void init(GLFWwindow* pWindow);

    void update();

    void showCursor();
    // Hides the cursor and stop reading the absolute position of the cursor
    bool hideCursor();

    inline const glm::dvec2& getMousePosition() const   { return m_MousePosition; };
    inline const glm::dvec2& getMouseMove() const       { return m_MouseMove; };

public:
    // Calls the stateful keyActionCallback
    static void keyActionCallbackStatic(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackStatic(GLFWwindow* pGLFWWindow, int button, int action, int mods);

private:
    void keyActionCallback(int key, int scancode, int action, int mods);
    void mouseButtonCallback(int button, int action, int mods);

private:
    GLFWwindow* m_pWindow;

    // Keyboard
    bool m_pKeyStates[GLFW_KEY_LAST + 1];

    // Mouse
    bool m_pMouseButtonStates[GLFW_MOUSE_BUTTON_LAST + 1];

    glm::dvec2 m_MousePosition;
    glm::dvec2 m_MouseMove;

    bool m_RawMotionEnabled;
};

#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputHandler
{
public:
    InputHandler();
    ~InputHandler() = default;

    void init(GLFWwindow* pWindow, uint32_t windowWidth, uint32_t windowHeight);

    void update();

    void enable()   { m_Enabled = true; }
    void disable();

    void showCursor();
    // Hides the cursor and stop reading the absolute position of the cursor
    bool hideCursor();

    bool cursorIsHidden() const     { return m_RawMotionEnabled; }

    bool keyState(int key) const    { return m_pKeyStates[key]; }

    const glm::dvec2& getMousePosition() const   { return m_MousePosition; };
    const glm::dvec2& getMouseMove() const       { return m_MouseMove; };

    bool mouseButtonState(int button) const { return m_pMouseButtonStates[button]; }

public:
    // Calls the stateful keyActionCallback
    static void keyActionCallbackStatic(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackStatic(GLFWwindow* pGLFWWindow, int button, int action, int mods);

private:
    void keyActionCallback(int key, int scancode, int action, int mods);
    void mouseButtonCallback(int button, int action, int mods);

private:
    GLFWwindow* m_pWindow;
    bool m_Enabled;

    // Keyboard
    std::array<bool, GLFW_KEY_LAST + 1> m_pKeyStates;

    // Mouse
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_pMouseButtonStates;

    // Center position of the screen. When raw mouse input is enabled, glfw returns the mouse position as mouseCenter + mouseMove.
    glm::dvec2 m_MouseCenter;

    glm::dvec2 m_MousePosition;
    glm::dvec2 m_MouseMove;

    bool m_RawMotionEnabled;
};

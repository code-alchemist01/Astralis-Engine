#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
#include <vector>

namespace Core {

enum class KeyState {
    Released = 0,
    Pressed = 1,
    Held = 2
};

enum class MouseButton {
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE
};

struct MousePosition {
    double x, y;
};

struct MouseDelta {
    double deltaX, deltaY;
};

// Input event callbacks
using KeyCallback = std::function<void(int key, KeyState state)>;
using MouseButtonCallback = std::function<void(MouseButton button, KeyState state)>;
using MouseMoveCallback = std::function<void(const MousePosition& position, const MouseDelta& delta)>;
using ScrollCallback = std::function<void(double xOffset, double yOffset)>;

class InputManager {
public:
    static InputManager& getInstance();
    
    // Initialize input system with GLFW window
    void initialize(GLFWwindow* window);
    void shutdown();
    
    // Update input state (call once per frame)
    void update();
    
    // Keyboard input
    bool isKeyPressed(int key) const;
    bool isKeyHeld(int key) const;
    bool isKeyReleased(int key) const;
    KeyState getKeyState(int key) const;
    
    // Mouse input
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonHeld(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    KeyState getMouseButtonState(MouseButton button) const;
    
    // Mouse position and movement
    MousePosition getMousePosition() const;
    MouseDelta getMouseDelta() const;
    void setMousePosition(double x, double y);
    
    // Mouse cursor mode
    void setCursorMode(int mode); // GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED
    int getCursorMode() const;
    
    // Event callbacks
    void setKeyCallback(const KeyCallback& callback);
    void setMouseButtonCallback(const MouseButtonCallback& callback);
    void setMouseMoveCallback(const MouseMoveCallback& callback);
    void setScrollCallback(const ScrollCallback& callback);
    
    // Clear callbacks
    void clearKeyCallback();
    void clearMouseButtonCallback();
    void clearMouseMoveCallback();
    void clearScrollCallback();

private:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    // GLFW callbacks (static functions)
    static void keyCallbackGLFW(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackGLFW(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallbackGLFW(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackGLFW(GLFWwindow* window, double xoffset, double yoffset);
    
    // Internal state management
    void updateKeyState(int key, int action);
    void updateMouseButtonState(int button, int action);
    void updateMousePosition(double x, double y);
    
    // Member variables
    GLFWwindow* m_window = nullptr;
    
    // Key states
    std::unordered_map<int, KeyState> m_keyStates;
    std::unordered_map<int, KeyState> m_previousKeyStates;
    
    // Mouse states
    std::unordered_map<int, KeyState> m_mouseButtonStates;
    std::unordered_map<int, KeyState> m_previousMouseButtonStates;
    
    // Mouse position
    MousePosition m_mousePosition{0.0, 0.0};
    MousePosition m_previousMousePosition{0.0, 0.0};
    MouseDelta m_mouseDelta{0.0, 0.0};
    
    // Event callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    ScrollCallback m_scrollCallback;
    
    // Initialization state
    bool m_initialized = false;
};

} // namespace Core
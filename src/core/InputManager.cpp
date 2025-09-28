#include "InputManager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Core {

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

void InputManager::initialize(GLFWwindow* window) {
    if (m_initialized) {
        spdlog::warn("InputManager already initialized");
        return;
    }
    
    m_window = window;
    
    // Set GLFW callbacks (don't override user pointer, use static instance)
    glfwSetKeyCallback(window, keyCallbackGLFW);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackGLFW);
    glfwSetCursorPosCallback(window, cursorPosCallbackGLFW);
    glfwSetScrollCallback(window, scrollCallbackGLFW);
    
    // Initialize mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    m_mousePosition = {xpos, ypos};
    m_previousMousePosition = m_mousePosition;
    
    m_initialized = true;
    spdlog::info("InputManager initialized");
}

void InputManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Clear callbacks
    if (m_window) {
        glfwSetKeyCallback(m_window, nullptr);
        glfwSetMouseButtonCallback(m_window, nullptr);
        glfwSetCursorPosCallback(m_window, nullptr);
        glfwSetScrollCallback(m_window, nullptr);
    }
    
    // Clear internal state
    m_keyStates.clear();
    m_previousKeyStates.clear();
    m_mouseButtonStates.clear();
    m_previousMouseButtonStates.clear();
    
    // Clear callbacks
    clearKeyCallback();
    clearMouseButtonCallback();
    clearMouseMoveCallback();
    clearScrollCallback();
    
    m_window = nullptr;
    m_initialized = false;
    
    spdlog::info("InputManager shutdown");
}

void InputManager::update() {
    if (!m_initialized) {
        return;
    }
    
    // Update previous states
    m_previousKeyStates = m_keyStates;
    m_previousMouseButtonStates = m_mouseButtonStates;
    m_previousMousePosition = m_mousePosition;
    
    // Update key states (convert PRESSED to HELD)
    for (auto& [key, state] : m_keyStates) {
        if (state == KeyState::Pressed) {
            state = KeyState::Held;
        }
    }
    
    // Update mouse button states (convert PRESSED to HELD)
    for (auto& [button, state] : m_mouseButtonStates) {
        if (state == KeyState::Pressed) {
            state = KeyState::Held;
        }
    }
    
    // Calculate mouse delta
    m_mouseDelta.deltaX = m_mousePosition.x - m_previousMousePosition.x;
    m_mouseDelta.deltaY = m_mousePosition.y - m_previousMousePosition.y;
}

// Keyboard input methods
bool InputManager::isKeyPressed(int key) const {
    return getKeyState(key) == KeyState::Pressed;
}

bool InputManager::isKeyHeld(int key) const {
    KeyState state = getKeyState(key);
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool InputManager::isKeyReleased(int key) const {
    auto current = m_keyStates.find(key);
    auto previous = m_previousKeyStates.find(key);
    
    KeyState currentState = (current != m_keyStates.end()) ? current->second : KeyState::Released;
    KeyState previousState = (previous != m_previousKeyStates.end()) ? previous->second : KeyState::Released;
    
    return currentState == KeyState::Released && 
           (previousState == KeyState::Pressed || previousState == KeyState::Held);
}

KeyState InputManager::getKeyState(int key) const {
    auto it = m_keyStates.find(key);
    return (it != m_keyStates.end()) ? it->second : KeyState::Released;
}

// Mouse input methods
bool InputManager::isMouseButtonPressed(MouseButton button) const {
    return getMouseButtonState(button) == KeyState::Pressed;
}

bool InputManager::isMouseButtonHeld(MouseButton button) const {
    KeyState state = getMouseButtonState(button);
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool InputManager::isMouseButtonReleased(MouseButton button) const {
    int buttonInt = static_cast<int>(button);
    auto current = m_mouseButtonStates.find(buttonInt);
    auto previous = m_previousMouseButtonStates.find(buttonInt);
    
    KeyState currentState = (current != m_mouseButtonStates.end()) ? current->second : KeyState::Released;
    KeyState previousState = (previous != m_previousMouseButtonStates.end()) ? previous->second : KeyState::Released;
    
    return currentState == KeyState::Released && 
           (previousState == KeyState::Pressed || previousState == KeyState::Held);
}

KeyState InputManager::getMouseButtonState(MouseButton button) const {
    int buttonInt = static_cast<int>(button);
    auto it = m_mouseButtonStates.find(buttonInt);
    return (it != m_mouseButtonStates.end()) ? it->second : KeyState::Released;
}

// Mouse position and movement
MousePosition InputManager::getMousePosition() const {
    return m_mousePosition;
}

MouseDelta InputManager::getMouseDelta() const {
    return m_mouseDelta;
}

void InputManager::setMousePosition(double x, double y) {
    if (m_window) {
        glfwSetCursorPos(m_window, x, y);
        m_mousePosition = {x, y};
    }
}

// Mouse cursor mode
void InputManager::setCursorMode(int mode) {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, mode);
    }
}

int InputManager::getCursorMode() const {
    if (m_window) {
        return glfwGetInputMode(m_window, GLFW_CURSOR);
    }
    return GLFW_CURSOR_NORMAL;
}

// Event callbacks
void InputManager::setKeyCallback(const KeyCallback& callback) {
    m_keyCallback = callback;
}

void InputManager::setMouseButtonCallback(const MouseButtonCallback& callback) {
    m_mouseButtonCallback = callback;
}

void InputManager::setMouseMoveCallback(const MouseMoveCallback& callback) {
    m_mouseMoveCallback = callback;
}

void InputManager::setScrollCallback(const ScrollCallback& callback) {
    m_scrollCallback = callback;
}

// Clear callbacks
void InputManager::clearKeyCallback() {
    m_keyCallback = nullptr;
}

void InputManager::clearMouseButtonCallback() {
    m_mouseButtonCallback = nullptr;
}

void InputManager::clearMouseMoveCallback() {
    m_mouseMoveCallback = nullptr;
}

void InputManager::clearScrollCallback() {
    m_scrollCallback = nullptr;
}

// GLFW callbacks (static functions)
void InputManager::keyCallbackGLFW(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager& inputManager = getInstance();
    if (inputManager.m_initialized) {
        inputManager.updateKeyState(key, action);
    }
}

void InputManager::mouseButtonCallbackGLFW(GLFWwindow* window, int button, int action, int mods) {
    InputManager& inputManager = getInstance();
    if (inputManager.m_initialized) {
        inputManager.updateMouseButtonState(button, action);
    }
}

void InputManager::cursorPosCallbackGLFW(GLFWwindow* window, double xpos, double ypos) {
    InputManager& inputManager = getInstance();
    if (inputManager.m_initialized) {
        inputManager.updateMousePosition(xpos, ypos);
    }
}

void InputManager::scrollCallbackGLFW(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager& inputManager = getInstance();
    if (inputManager.m_initialized && inputManager.m_scrollCallback) {
        inputManager.m_scrollCallback(xoffset, yoffset);
    }
}

// Internal state management
void InputManager::updateKeyState(int key, int action) {
    KeyState newState;
    
    switch (action) {
        case GLFW_PRESS:
            newState = KeyState::Pressed;
            break;
        case GLFW_RELEASE:
            newState = KeyState::Released;
            break;
        case GLFW_REPEAT:
            newState = KeyState::Held;
            break;
        default:
            return;
    }
    
    m_keyStates[key] = newState;
    
    // Call user callback if set
    if (m_keyCallback) {
        m_keyCallback(key, newState);
    }
}

void InputManager::updateMouseButtonState(int button, int action) {
    KeyState newState;
    
    switch (action) {
        case GLFW_PRESS:
            newState = KeyState::Pressed;
            break;
        case GLFW_RELEASE:
            newState = KeyState::Released;
            break;
        default:
            return;
    }
    
    m_mouseButtonStates[button] = newState;
    
    // Call user callback if set
    if (m_mouseButtonCallback) {
        MouseButton mouseButton = static_cast<MouseButton>(button);
        m_mouseButtonCallback(mouseButton, newState);
    }
}

void InputManager::updateMousePosition(double x, double y) {
    MousePosition oldPosition = m_mousePosition;
    m_mousePosition = {x, y};
    
    // Call user callback if set
    if (m_mouseMoveCallback) {
        MouseDelta delta = {x - oldPosition.x, y - oldPosition.y};
        m_mouseMoveCallback(m_mousePosition, delta);
    }
}

} // namespace Core
#include "Window.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

Window::Window(int width, int height, const std::string& title) {
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }
    
    // Set OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Additional hints
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
    
    // Create window
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }
    
    // Make context current
    glfwMakeContextCurrent(window_);
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);
    
    // Set only framebuffer callback (InputManager will handle input callbacks)
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    
    spdlog::info("Window created: {}x{}", width, height);
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(window_);
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(window_, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

void Window::getCursorPos(double& x, double& y) const {
    glfwGetCursorPos(window_, &x, &y);
}

void Window::getSize(int& width, int& height) const {
    glfwGetWindowSize(window_, &width, &height);
}

int Window::getWidth() const {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return width;
}

int Window::getHeight() const {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return height;
}

void Window::setSize(int width, int height) {
    glfwSetWindowSize(window_, width, height);
}

float Window::getAspectRatio() const {
    int width, height;
    getSize(width, height);
    return static_cast<float>(width) / static_cast<float>(height);
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
    resizeCallback_ = std::move(callback);
}

void Window::setKeyCallback(std::function<void(int, int, int, int)> callback) {
    keyCallback_ = std::move(callback);
}

void Window::setMouseButtonCallback(std::function<void(int, int, int)> callback) {
    mouseButtonCallback_ = std::move(callback);
}

void Window::setCursorPosCallback(std::function<void(double, double)> callback) {
    cursorPosCallback_ = std::move(callback);
}

// Static callback functions
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->resizeCallback_) {
        win->resizeCallback_(width, height);
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->keyCallback_) {
        win->keyCallback_(key, scancode, action, mods);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->mouseButtonCallback_) {
        win->mouseButtonCallback_(button, action, mods);
    }
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->cursorPosCallback_) {
        win->cursorPosCallback_(xpos, ypos);
    }
}
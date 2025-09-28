#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Non-copyable, non-movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool isValid() const { return window_ != nullptr; }
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    
    // Get GLFW window pointer (for InputManager)
    GLFWwindow* getGLFWwindow() const { return window_; }
    
    // Input
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getCursorPos(double& x, double& y) const;
    
    // Properties
    void getSize(int& width, int& height) const;
    int getWidth() const;
    int getHeight() const;
    void setSize(int width, int height);
    float getAspectRatio() const;
    
    // Callbacks
    void setResizeCallback(std::function<void(int, int)> callback);
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    void setMouseButtonCallback(std::function<void(int, int, int)> callback);
    void setCursorPosCallback(std::function<void(double, double)> callback);

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    GLFWwindow* window_ = nullptr;
    
    // Callback functions
    std::function<void(int, int)> resizeCallback_;
    std::function<void(int, int, int, int)> keyCallback_;
    std::function<void(int, int, int)> mouseButtonCallback_;
    std::function<void(double, double)> cursorPosCallback_;
};
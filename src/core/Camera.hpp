#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // Camera movement directions
    enum class Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    // Destructor
    ~Camera() = default;

    // Get view matrix
    glm::mat4 getViewMatrix() const;

    // Get projection matrix
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    // Process keyboard input
    void processKeyboard(Movement direction, float deltaTime);

    // Process mouse movement
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

    // Process mouse scroll
    void processMouseScroll(float yOffset);

    // Getters
    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getFront() const { return front_; }
    glm::vec3 getUp() const { return up_; }
    glm::vec3 getRight() const { return right_; }
    float getZoom() const { return zoom_; }
    float getYaw() const { return yaw_; }
    float getPitch() const { return pitch_; }

    // Setters
    void setPosition(const glm::vec3& position) { position_ = position; }
    void setMovementSpeed(float speed) { movementSpeed_ = speed; }
    void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
    void setZoom(float zoom) { zoom_ = zoom; }
    void setYaw(float yaw) { yaw_ = yaw; updateCameraVectors(); }
    void setPitch(float pitch) { pitch_ = pitch; updateCameraVectors(); }

private:
    // Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    // Euler angles
    float yaw_;
    float pitch_;

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoom_;

    // Projection settings
    float nearPlane_;
    float farPlane_;

    // Update camera vectors based on updated Euler angles
    void updateCameraVectors();
};
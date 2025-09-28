#include "Camera.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

// Default camera values
const float DEFAULT_YAW = -90.0f;
const float DEFAULT_PITCH = 0.0f;
const float DEFAULT_SPEED = 25.0f;  // Increased for better navigation
const float DEFAULT_SENSITIVITY = 0.3f;
const float DEFAULT_ZOOM = 45.0f;
const float DEFAULT_NEAR_PLANE = 0.1f;
const float DEFAULT_FAR_PLANE = 10000.0f;  // Increased for distant viewing

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position_(position)
    , worldUp_(up)
    , yaw_(yaw)
    , pitch_(pitch)
    , movementSpeed_(DEFAULT_SPEED)
    , mouseSensitivity_(DEFAULT_SENSITIVITY)
    , zoom_(DEFAULT_ZOOM)
    , nearPlane_(DEFAULT_NEAR_PLANE)
    , farPlane_(DEFAULT_FAR_PLANE)
{
    updateCameraVectors();
    spdlog::info("Camera created at position: ({}, {}, {})", position_.x, position_.y, position_.z);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(zoom_), aspectRatio, nearPlane_, farPlane_);
}

void Camera::processKeyboard(Movement direction, float deltaTime) {
    float velocity = movementSpeed_ * deltaTime;
    
    switch (direction) {
        case Movement::FORWARD:
            position_ += front_ * velocity;
            break;
        case Movement::BACKWARD:
            position_ -= front_ * velocity;
            break;
        case Movement::LEFT:
            position_ -= right_ * velocity;
            break;
        case Movement::RIGHT:
            position_ += right_ * velocity;
            break;
        case Movement::UP:
            position_ += up_ * velocity;
            break;
        case Movement::DOWN:
            position_ -= up_ * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity_;
    yOffset *= mouseSensitivity_;

    yaw_ += xOffset;
    pitch_ += yOffset;

    // Constrain pitch to avoid screen flip
    if (constrainPitch) {
        pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    }

    // Update front, right and up vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    zoom_ -= yOffset;
    zoom_ = std::clamp(zoom_, 1.0f, 45.0f);
}

void Camera::updateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    
    // Re-calculate the right and up vector
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
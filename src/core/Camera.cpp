#include "Camera.hpp"
#include "Planet.hpp"
#include "Sun.hpp"
#include <algorithm>
#include <cmath>

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 10000.0f;

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position_(position)
    , currentPosition_(position)
    , front_(glm::vec3(0.0f, 0.0f, -1.0f))
    , worldUp_(up)
    , velocity_(glm::vec3(0.0f))
    , yaw_(yaw)
    , pitch_(pitch)
    , movementSpeed_(SPEED)
    , mouseSensitivity_(SENSITIVITY)
    , zoom_(ZOOM)
    , nearPlane_(NEAR_PLANE)
    , farPlane_(FAR_PLANE)
    , currentMode_(Mode::FREE_FLY)
    , target_(glm::vec3(0.0f))
    , targetPlanet_(nullptr)
    , targetSun_(nullptr)
    , transitioning_(false)
    , transitionDuration_(0.0f)
    , transitionTime_(0.0f)
    , transitionType_(TransitionType::LINEAR)
    , autoFollowEnabled_(false)
    , followDistance_(100.0f)
    , followHeight_(20.0f)
    , followSmoothing_(2.0f)
    , orbitDistance_(200.0f)
    , orbitSpeed_(1.0f)
    , orbitHeight_(0.0f)
    , orbitAngle_(0.0f)
    , speedMultiplier_(1.0f)
    , speedBoostEnabled_(false)
    , boostMultiplier_(5.0f)
    , cinematicPlaying_(false)
    , cinematicTime_(0.0f)
    , cinematicDuration_(0.0f)
    , shakeEnabled_(false)
    , shakeIntensity_(0.0f)
    , shakeDuration_(0.0f)
    , shakeTime_(0.0f)
    , shakeOffset_(glm::vec3(0.0f))
    , motionBlurEnabled_(false) {
    updateCameraVectors();
    saveCurrentState();
}

glm::mat4 Camera::getViewMatrix() const {
    glm::vec3 viewPosition = currentPosition_ + shakeOffset_;
    return glm::lookAt(viewPosition, viewPosition + front_, up_);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(zoom_), aspectRatio, nearPlane_, farPlane_);
}

void Camera::update(float deltaTime) {
    updateTransition(deltaTime);
    updateMode(deltaTime);
    updateShake(deltaTime);
    updateCinematic(deltaTime);
    
    // Update velocity for motion blur
    velocity_ = (currentPosition_ - position_) / deltaTime;
    position_ = currentPosition_;
}

void Camera::processKeyboard(Movement direction, float deltaTime) {
    if (cinematicPlaying_ || transitioning_) return;
    
    float velocity = movementSpeed_ * speedMultiplier_ * deltaTime;
    if (speedBoostEnabled_) {
        velocity *= boostMultiplier_;
    }
    
    switch (direction) {
        case Movement::FORWARD:
            currentPosition_ += front_ * velocity;
            break;
        case Movement::BACKWARD:
            currentPosition_ -= front_ * velocity;
            break;
        case Movement::LEFT:
            currentPosition_ -= right_ * velocity;
            break;
        case Movement::RIGHT:
            currentPosition_ += right_ * velocity;
            break;
        case Movement::UP:
            currentPosition_ += up_ * velocity;
            break;
        case Movement::DOWN:
            currentPosition_ -= up_ * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    if (cinematicPlaying_) return;
    
    xOffset *= mouseSensitivity_;
    yOffset *= mouseSensitivity_;

    yaw_ += xOffset;
    pitch_ += yOffset;

    // Constrain pitch to avoid screen flip
    if (constrainPitch) {
        if (pitch_ > 89.0f)
            pitch_ = 89.0f;
        if (pitch_ < -89.0f)
            pitch_ = -89.0f;
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    zoom_ -= yOffset;
    zoom_ = std::clamp(zoom_, 1.0f, 45.0f);
}

void Camera::setMode(Mode mode) {
    currentMode_ = mode;
    
    // Reset mode-specific variables
    orbitAngle_ = 0.0f;
    cinematicPlaying_ = false;
}

void Camera::setTarget(const glm::vec3& target) {
    target_ = target;
    targetPlanet_ = nullptr;
    targetSun_ = nullptr;
}

void Camera::setTargetPlanet(Planet* planet) {
    targetPlanet_ = planet;
    targetSun_ = nullptr;
    if (planet) {
        // target_ = planet->getPosition(); // Uncomment when Planet class has getPosition()
    }
}

void Camera::setTargetSun(Sun* sun) {
    targetSun_ = sun;
    targetPlanet_ = nullptr;
    if (sun) {
        target_ = glm::vec3(0.0f); // Sun is typically at origin
    }
}

void Camera::setPosition(const glm::vec3& position) {
    position_ = position;
    currentPosition_ = position;
}

void Camera::transitionToPosition(const glm::vec3& newPosition, float duration, TransitionType type) {
    transitionStart_ = currentPosition_;
    transitionEnd_ = newPosition;
    transitionDuration_ = duration;
    transitionTime_ = 0.0f;
    transitionType_ = type;
    transitioning_ = true;
}

void Camera::transitionToTarget(const glm::vec3& target, float distance, float duration, TransitionType type) {
    glm::vec3 direction = glm::normalize(currentPosition_ - target);
    glm::vec3 newPosition = target + direction * distance;
    transitionToPosition(newPosition, duration, type);
    
    // Also look at the target
    lookAtTarget(target, duration * 0.5f);
}

void Camera::lookAtTarget(const glm::vec3& target, float duration) {
    glm::vec3 direction = glm::normalize(target - currentPosition_);
    
    // Calculate target yaw and pitch
    float targetYaw = glm::degrees(atan2(direction.z, direction.x));
    float targetPitch = glm::degrees(asin(direction.y));
    
    // Smooth transition to new angles (simplified for now)
    yaw_ = targetYaw;
    pitch_ = targetPitch;
    updateCameraVectors();
}

void Camera::startCinematicPath(const std::vector<glm::vec3>& waypoints, float totalDuration) {
    cinematicKeyframes_.clear();
    cinematicDuration_ = totalDuration;
    
    float timeStep = totalDuration / (waypoints.size() - 1);
    for (size_t i = 0; i < waypoints.size(); ++i) {
        CinematicKeyframe keyframe;
        keyframe.position = waypoints[i];
        keyframe.lookAt = (i < waypoints.size() - 1) ? waypoints[i + 1] : waypoints[i];
        keyframe.time = i * timeStep;
        cinematicKeyframes_.push_back(keyframe);
    }
}

void Camera::addCinematicKeyframe(const glm::vec3& position, const glm::vec3& lookAt, float time) {
    CinematicKeyframe keyframe;
    keyframe.position = position;
    keyframe.lookAt = lookAt;
    keyframe.time = time;
    cinematicKeyframes_.push_back(keyframe);
    
    // Sort by time
    std::sort(cinematicKeyframes_.begin(), cinematicKeyframes_.end(),
              [](const CinematicKeyframe& a, const CinematicKeyframe& b) {
                  return a.time < b.time;
              });
    
    if (!cinematicKeyframes_.empty()) {
        cinematicDuration_ = cinematicKeyframes_.back().time;
    }
}

void Camera::playCinematicSequence() {
    if (!cinematicKeyframes_.empty()) {
        cinematicPlaying_ = true;
        cinematicTime_ = 0.0f;
        setMode(Mode::CINEMATIC);
    }
}

void Camera::stopCinematicSequence() {
    cinematicPlaying_ = false;
    setMode(Mode::FREE_FLY);
}

void Camera::addCameraShake(float intensity, float duration) {
    shakeEnabled_ = true;
    shakeIntensity_ = intensity;
    shakeDuration_ = duration;
    shakeTime_ = 0.0f;
}

float Camera::getDistanceToTarget() const {
    return glm::length(currentPosition_ - target_);
}

void Camera::resetToDefault() {
    currentPosition_ = glm::vec3(0.0f, 0.0f, 3.0f);
    position_ = currentPosition_;
    yaw_ = YAW;
    pitch_ = PITCH;
    zoom_ = ZOOM;
    currentMode_ = Mode::FREE_FLY;
    transitioning_ = false;
    cinematicPlaying_ = false;
    shakeEnabled_ = false;
    updateCameraVectors();
}

void Camera::saveCurrentState() {
    savedState_.position = currentPosition_;
    savedState_.yaw = yaw_;
    savedState_.pitch = pitch_;
    savedState_.zoom = zoom_;
    savedState_.mode = currentMode_;
}

void Camera::restoreSavedState() {
    setPosition(savedState_.position);
    yaw_ = savedState_.yaw;
    pitch_ = savedState_.pitch;
    zoom_ = savedState_.zoom;
    setMode(savedState_.mode);
    updateCameraVectors();
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

void Camera::updateTransition(float deltaTime) {
    if (!transitioning_) return;
    
    transitionTime_ += deltaTime;
    float t = std::min(transitionTime_ / transitionDuration_, 1.0f);
    
    currentPosition_ = interpolatePosition(transitionStart_, transitionEnd_, t, transitionType_);
    
    if (t >= 1.0f) {
        transitioning_ = false;
        if (transitionCallback_) {
            transitionCallback_();
            transitionCallback_ = nullptr;
        }
    }
}

void Camera::updateAutoFollow(float deltaTime) {
    if (!autoFollowEnabled_ || currentMode_ != Mode::FOLLOW) return;
    
    glm::vec3 targetPos = target_;
    if (targetPlanet_) {
        // targetPos = targetPlanet_->getPosition(); // Uncomment when available
    }
    
    glm::vec3 desiredPosition = targetPos + glm::vec3(0.0f, followHeight_, followDistance_);
    currentPosition_ = glm::mix(currentPosition_, desiredPosition, followSmoothing_ * deltaTime);
    
    // Look at target
    glm::vec3 direction = glm::normalize(targetPos - currentPosition_);
    yaw_ = glm::degrees(atan2(direction.z, direction.x));
    pitch_ = glm::degrees(asin(direction.y));
    updateCameraVectors();
}

void Camera::updateOrbit(float deltaTime) {
    if (currentMode_ != Mode::ORBIT) return;
    
    orbitAngle_ += orbitSpeed_ * deltaTime;
    
    glm::vec3 targetPos = target_;
    if (targetPlanet_) {
        // targetPos = targetPlanet_->getPosition(); // Uncomment when available
    }
    
    float x = targetPos.x + cos(orbitAngle_) * orbitDistance_;
    float z = targetPos.z + sin(orbitAngle_) * orbitDistance_;
    float y = targetPos.y + orbitHeight_;
    
    currentPosition_ = glm::vec3(x, y, z);
    
    // Look at target
    glm::vec3 direction = glm::normalize(targetPos - currentPosition_);
    yaw_ = glm::degrees(atan2(direction.z, direction.x));
    pitch_ = glm::degrees(asin(direction.y));
    updateCameraVectors();
}

void Camera::updateCinematic(float deltaTime) {
    if (!cinematicPlaying_ || cinematicKeyframes_.empty()) return;
    
    cinematicTime_ += deltaTime;
    
    if (cinematicTime_ >= cinematicDuration_) {
        cinematicPlaying_ = false;
        return;
    }
    
    glm::vec3 position, lookAt;
    calculateCinematicPosition(cinematicTime_, position, lookAt);
    
    currentPosition_ = position;
    
    // Look at target
    glm::vec3 direction = glm::normalize(lookAt - position);
    yaw_ = glm::degrees(atan2(direction.z, direction.x));
    pitch_ = glm::degrees(asin(direction.y));
    updateCameraVectors();
}

void Camera::updateShake(float deltaTime) {
    if (!shakeEnabled_) {
        shakeOffset_ = glm::vec3(0.0f);
        return;
    }
    
    shakeTime_ += deltaTime;
    
    if (shakeTime_ >= shakeDuration_) {
        shakeEnabled_ = false;
        shakeOffset_ = glm::vec3(0.0f);
        return;
    }
    
    float intensity = shakeIntensity_ * (1.0f - shakeTime_ / shakeDuration_);
    shakeOffset_.x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * intensity;
    shakeOffset_.y = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * intensity;
    shakeOffset_.z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * intensity;
}

void Camera::updateMode(float deltaTime) {
    switch (currentMode_) {
        case Mode::FOLLOW:
            updateAutoFollow(deltaTime);
            break;
        case Mode::ORBIT:
            updateOrbit(deltaTime);
            break;
        case Mode::CINEMATIC:
            updateCinematic(deltaTime);
            break;
        default:
            break;
    }
}

float Camera::interpolate(float start, float end, float t, TransitionType type) {
    switch (type) {
        case TransitionType::LINEAR:
            return start + (end - start) * t;
        case TransitionType::EASE_IN:
            return start + (end - start) * t * t;
        case TransitionType::EASE_OUT:
            return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
        case TransitionType::EASE_IN_OUT:
            if (t < 0.5f) {
                return start + (end - start) * 2.0f * t * t;
            } else {
                return start + (end - start) * (1.0f - 2.0f * (1.0f - t) * (1.0f - t));
            }
        case TransitionType::SMOOTH_STEP:
            t = t * t * (3.0f - 2.0f * t);
            return start + (end - start) * t;
        default:
            return start + (end - start) * t;
    }
}

glm::vec3 Camera::interpolatePosition(const glm::vec3& start, const glm::vec3& end, float t, TransitionType type) {
    return glm::vec3(
        interpolate(start.x, end.x, t, type),
        interpolate(start.y, end.y, t, type),
        interpolate(start.z, end.z, t, type)
    );
}

void Camera::calculateCinematicPosition(float time, glm::vec3& position, glm::vec3& lookAt) {
    if (cinematicKeyframes_.size() < 2) {
        if (!cinematicKeyframes_.empty()) {
            position = cinematicKeyframes_[0].position;
            lookAt = cinematicKeyframes_[0].lookAt;
        }
        return;
    }
    
    // Find the two keyframes to interpolate between
    size_t index = 0;
    for (size_t i = 0; i < cinematicKeyframes_.size() - 1; ++i) {
        if (time >= cinematicKeyframes_[i].time && time <= cinematicKeyframes_[i + 1].time) {
            index = i;
            break;
        }
    }
    
    const auto& keyframe1 = cinematicKeyframes_[index];
    const auto& keyframe2 = cinematicKeyframes_[index + 1];
    
    float t = (time - keyframe1.time) / (keyframe2.time - keyframe1.time);
    t = std::clamp(t, 0.0f, 1.0f);
    
    position = interpolatePosition(keyframe1.position, keyframe2.position, t, TransitionType::SMOOTH_STEP);
    lookAt = interpolatePosition(keyframe1.lookAt, keyframe2.lookAt, t, TransitionType::SMOOTH_STEP);
}
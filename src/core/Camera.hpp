#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <memory>

// Forward declarations
class Planet;
class Sun;

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

    // Camera modes for different behaviors
    enum class Mode {
        FREE_FLY,           // Standard free-flying camera
        ORBIT,              // Orbit around a target
        FOLLOW,             // Follow a target at distance
        CINEMATIC,          // Smooth cinematic movement
        FIRST_PERSON,       // First person view (for spacecraft)
        PLANETARY_SURFACE   // Surface exploration mode
    };

    // Transition types for smooth camera movements
    enum class TransitionType {
        LINEAR,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT,
        SMOOTH_STEP
    };

    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    // Destructor
    ~Camera() = default;

    // Core camera functions
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    void update(float deltaTime);

    // Input processing
    void processKeyboard(Movement direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);

    // Enhanced camera controls
    void setMode(Mode mode);
    void setTarget(const glm::vec3& target);
    void setTargetPlanet(Planet* planet);
    void setTargetSun(Sun* sun);
    
    // Smooth transitions
    void transitionToPosition(const glm::vec3& newPosition, float duration = 2.0f, 
                             TransitionType type = TransitionType::EASE_IN_OUT);
    void transitionToTarget(const glm::vec3& target, float distance = 100.0f, 
                           float duration = 2.0f, TransitionType type = TransitionType::EASE_IN_OUT);
    void lookAtTarget(const glm::vec3& target, float duration = 1.0f);

    // Cinematic features
    void startCinematicPath(const std::vector<glm::vec3>& waypoints, float totalDuration);
    void addCinematicKeyframe(const glm::vec3& position, const glm::vec3& lookAt, float time);
    void playCinematicSequence();
    void stopCinematicSequence();
    bool isCinematicPlaying() const { return cinematicPlaying_; }

    // Auto-follow features
    void enableAutoFollow(bool enable) { autoFollowEnabled_ = enable; }
    void setFollowDistance(float distance) { followDistance_ = distance; }
    void setFollowHeight(float height) { followHeight_ = height; }
    void setFollowSmoothing(float smoothing) { followSmoothing_ = smoothing; }

    // Orbit controls
    void setOrbitDistance(float distance) { orbitDistance_ = distance; }
    void setOrbitSpeed(float speed) { orbitSpeed_ = speed; }
    void setOrbitHeight(float height) { orbitHeight_ = height; }

    // Speed controls
    void setSpeedMultiplier(float multiplier) { speedMultiplier_ = multiplier; }
    void enableSpeedBoost(bool enable) { speedBoostEnabled_ = enable; }
    void setBoostMultiplier(float multiplier) { boostMultiplier_ = multiplier; }

    // Shake and effects
    void addCameraShake(float intensity, float duration);
    void enableMotionBlur(bool enable) { motionBlurEnabled_ = enable; }

    // Getters
    glm::vec3 getPosition() const { return currentPosition_; }
    glm::vec3 getFront() const { return front_; }
    glm::vec3 getUp() const { return up_; }
    glm::vec3 getRight() const { return right_; }
    float getZoom() const { return zoom_; }
    float getYaw() const { return yaw_; }
    float getPitch() const { return pitch_; }
    Mode getMode() const { return currentMode_; }
    glm::vec3 getTarget() const { return target_; }
    float getDistanceToTarget() const;
    glm::vec3 getVelocity() const { return velocity_; }

    // Setters
    void setPosition(const glm::vec3& position);
    void setMovementSpeed(float speed) { movementSpeed_ = speed; }
    void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
    void setZoom(float zoom) { zoom_ = zoom; }
    void setYaw(float yaw) { yaw_ = yaw; updateCameraVectors(); }
    void setPitch(float pitch) { pitch_ = pitch; updateCameraVectors(); }

    // Utility functions
    void resetToDefault();
    void saveCurrentState();
    void restoreSavedState();
    bool isTransitioning() const { return transitioning_; }

private:
    // Core camera attributes
    glm::vec3 position_;
    glm::vec3 currentPosition_;  // For smooth transitions
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;
    glm::vec3 velocity_;

    // Euler angles
    float yaw_;
    float pitch_;

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoom_;
    float nearPlane_;
    float farPlane_;

    // Enhanced features
    Mode currentMode_;
    glm::vec3 target_;
    Planet* targetPlanet_;
    Sun* targetSun_;

    // Transition system
    bool transitioning_;
    glm::vec3 transitionStart_;
    glm::vec3 transitionEnd_;
    float transitionDuration_;
    float transitionTime_;
    TransitionType transitionType_;
    std::function<void()> transitionCallback_;

    // Auto-follow system
    bool autoFollowEnabled_;
    float followDistance_;
    float followHeight_;
    float followSmoothing_;

    // Orbit system
    float orbitDistance_;
    float orbitSpeed_;
    float orbitHeight_;
    float orbitAngle_;

    // Speed system
    float speedMultiplier_;
    bool speedBoostEnabled_;
    float boostMultiplier_;

    // Cinematic system
    struct CinematicKeyframe {
        glm::vec3 position;
        glm::vec3 lookAt;
        float time;
    };
    std::vector<CinematicKeyframe> cinematicKeyframes_;
    bool cinematicPlaying_;
    float cinematicTime_;
    float cinematicDuration_;

    // Camera shake
    bool shakeEnabled_;
    float shakeIntensity_;
    float shakeDuration_;
    float shakeTime_;
    glm::vec3 shakeOffset_;

    // Effects
    bool motionBlurEnabled_;

    // Saved state
    struct CameraState {
        glm::vec3 position;
        float yaw;
        float pitch;
        float zoom;
        Mode mode;
    };
    CameraState savedState_;

    // Private methods
    void updateCameraVectors();
    void updateTransition(float deltaTime);
    void updateAutoFollow(float deltaTime);
    void updateOrbit(float deltaTime);
    void updateCinematic(float deltaTime);
    void updateShake(float deltaTime);
    void updateMode(float deltaTime);
    
    // Interpolation functions
    float interpolate(float start, float end, float t, TransitionType type);
    glm::vec3 interpolatePosition(const glm::vec3& start, const glm::vec3& end, float t, TransitionType type);
    
    // Cinematic helpers
    void calculateCinematicPosition(float time, glm::vec3& position, glm::vec3& lookAt);
};
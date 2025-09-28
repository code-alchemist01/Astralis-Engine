#pragma once

#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class Geometry;
class Shader;
class Camera;

/**
 * @brief Sun class representing the central star of a solar system
 */
class Sun {
public:
    /**
     * @brief Construct a new Sun object
     * @param position Position of the sun in world space
     * @param radius Radius of the sun
     * @param color Base color of the sun
     * @param temperature Surface temperature (affects color)
     * @param intensity Light intensity emitted by the sun
     */
    Sun(const glm::vec3& position = glm::vec3(0.0f), 
        float radius = 10.0f, 
        const glm::vec3& color = glm::vec3(1.0f, 0.8f, 0.4f),
        float temperature = 5778.0f,  // Sun's surface temperature in Kelvin
        float intensity = 1.0f);

    /**
     * @brief Destroy the Sun object
     */
    ~Sun() = default;

    // Non-copyable, non-movable for now
    Sun(const Sun&) = delete;
    Sun& operator=(const Sun&) = delete;
    Sun(Sun&&) = delete;
    Sun& operator=(Sun&&) = delete;

    /**
     * @brief Initialize the sun geometry and resources
     * @param resolution Sphere resolution for the sun mesh
     */
    void initialize(int resolution = 64);

    /**
     * @brief Initialize the sun with specific parameters
     * @param position Position of the sun
     * @param radius Radius of the sun
     * @param temperature Surface temperature in Kelvin
     * @param intensity Light intensity
     * @param resolution Sphere resolution for the sun mesh
     */
    void initialize(const glm::vec3& position, float radius, float temperature, float intensity, int resolution = 64);

    /**
     * @brief Update sun animation (rotation, pulsing, etc.)
     * @param deltaTime Time elapsed since last frame
     */
    void update(float deltaTime);

    /**
     * @brief Render the sun with glowing effects
     * @param shader Shader program to use for rendering
     * @param camera Camera for view calculations
     * @param view View matrix
     * @param projection Projection matrix
     */
    void render(Shader* shader, const Camera* camera, 
                const glm::mat4& view, const glm::mat4& projection);

    // Getters
    const glm::vec3& getPosition() const { return position_; }
    float getRadius() const { return radius_; }
    const glm::vec3& getColor() const { return color_; }
    float getTemperature() const { return temperature_; }
    float getIntensity() const { return intensity_; }
    float getCurrentRotation() const { return currentRotation_; }

    // Setters
    void setPosition(const glm::vec3& position) { position_ = position; }
    void setRadius(float radius) { radius_ = radius; }
    void setColor(const glm::vec3& color) { color_ = color; }
    void setTemperature(float temperature) { temperature_ = temperature; }
    void setIntensity(float intensity) { intensity_ = intensity; }

    /**
     * @brief Calculate light properties for other objects
     * @return Light position, color, and intensity for lighting calculations
     */
    struct LightProperties {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
    LightProperties getLightProperties() const;

private:
    glm::vec3 position_;
    float radius_;
    glm::vec3 color_;
    float temperature_;
    float intensity_;
    
    // Animation properties
    float currentRotation_;
    float rotationSpeed_;
    float pulsePhase_;
    float pulseIntensity_;
    
    // Rendering resources
    std::unique_ptr<Geometry> geometry_;
    
    /**
     * @brief Calculate color based on temperature (blackbody radiation)
     * @param temperature Temperature in Kelvin
     * @return RGB color based on temperature
     */
    glm::vec3 calculateTemperatureColor(float temperature) const;
};
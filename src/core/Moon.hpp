#pragma once

#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class Planet;
class Shader;
class Camera;

/**
 * @brief Moon class representing a celestial body that orbits around a planet
 */
class Moon {
public:
    /**
     * @brief Construct a new Moon object
     * @param radius Moon radius in world units
     * @param orbitRadius Distance from the planet center
     * @param orbitSpeed Angular velocity around the planet
     * @param color Moon color
     * @param resolution Geometry resolution
     */
    Moon(float radius = 5.0f, float orbitRadius = 20.0f, float orbitSpeed = 1.0f, 
         const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f), int resolution = 16);

    /**
     * @brief Destroy the Moon object
     */
    ~Moon() = default;

    // Non-copyable, non-movable for now
    Moon(const Moon&) = delete;
    Moon& operator=(const Moon&) = delete;
    Moon(Moon&&) = delete;
    Moon& operator=(Moon&&) = delete;

    /**
     * @brief Update moon's orbital position
     * @param deltaTime Time elapsed since last update
     * @param planetPosition Current position of the parent planet
     */
    void update(float deltaTime, const glm::vec3& planetPosition);

    /**
     * @brief Render the moon
     * @param shader Shader program to use for rendering
     * @param camera Camera for view calculations
     * @param view View matrix
     * @param projection Projection matrix
     * @param lightPos Light position for shading
     * @param lightColor Light color
     * @param viewPos Camera position
     */
    void render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                const glm::mat4& projection, const glm::vec3& lightPos, 
                const glm::vec3& lightColor, const glm::vec3& viewPos);

    /**
     * @brief Get current world position of the moon
     * @return Current position in world coordinates
     */
    glm::vec3 getPosition() const { return position_; }

    /**
     * @brief Get moon radius
     * @return Moon radius
     */
    float getRadius() const { return radius_; }

    /**
     * @brief Get moon color
     * @return Moon color
     */
    glm::vec3 getColor() const { return color_; }

    /**
     * @brief Set moon color
     * @param color New color
     */
    void setColor(const glm::vec3& color) { color_ = color; }

private:
    std::unique_ptr<Planet> planet_;    ///< Moon geometry (using Planet class for sphere)
    glm::vec3 position_;                ///< Current world position
    glm::vec3 color_;                   ///< Moon color
    float radius_;                      ///< Moon radius
    float orbitRadius_;                 ///< Distance from planet center
    float orbitSpeed_;                  ///< Angular velocity (radians per second)
    float currentOrbitAngle_;           ///< Current orbital angle
    float orbitInclination_;            ///< Orbital plane inclination
    float rotationSpeed_;               ///< Moon's rotation around its axis
    float currentRotation_;             ///< Current rotation angle
};
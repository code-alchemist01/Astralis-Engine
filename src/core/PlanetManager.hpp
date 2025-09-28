#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Moon.hpp"

// Forward declarations
class Planet;
class Noise;
class Shader;
class Camera;

/**
 * @brief Structure to hold planet instance data with orbital mechanics
 */
struct PlanetInstance {
    std::unique_ptr<Planet> planet;
    glm::vec3 position;
    float scale;
    glm::vec3 color;
    float rotationSpeed;
    float currentRotation;
    int seed;
    int type; // 0=rocky, 1=gas, 2=ice, 3=desert
    
    // Orbital mechanics properties
    float orbitRadius;          // Distance from the sun
    float orbitSpeed;           // Angular velocity around the sun
    float currentOrbitAngle;    // Current position in orbit (radians)
    glm::vec3 orbitCenter;      // Center of orbit (usually sun position)
    float orbitInclination;     // Orbital plane inclination
    float orbitEccentricity;    // Orbit ellipse eccentricity (0 = circle, <1 = ellipse)
    
    // Moon system
    std::vector<std::unique_ptr<Moon>> moons;  // Moons orbiting this planet
    
    PlanetInstance(std::unique_ptr<Planet> p, glm::vec3 pos, float s, glm::vec3 col, float rotSpeed, int planetSeed, int planetType = 0)
        : planet(std::move(p)), position(pos), scale(s), color(col), 
          rotationSpeed(rotSpeed), currentRotation(0.0f), seed(planetSeed), type(planetType),
          orbitRadius(glm::length(pos)), orbitSpeed(0.1f), currentOrbitAngle(0.0f),
          orbitCenter(glm::vec3(0.0f)), orbitInclination(0.0f), orbitEccentricity(0.0f) {}
};

/**
 * @brief Manager class for handling multiple planets in the universe
 */
class PlanetManager {
public:
    /**
     * @brief Construct a new Planet Manager object
     */
    PlanetManager();

    /**
     * @brief Destroy the Planet Manager object
     */
    ~PlanetManager() = default;

    // Non-copyable, non-movable for now
    PlanetManager(const PlanetManager&) = delete;
    PlanetManager& operator=(const PlanetManager&) = delete;
    PlanetManager(PlanetManager&&) = delete;
    PlanetManager& operator=(PlanetManager&&) = delete;

    /**
     * @brief Initialize the planet manager with noise generator
     * @param noise Shared noise generator for all planets
     */
    void initialize(Noise* noise);

    /**
     * @brief Generate a solar system with multiple planets
     * @param systemSeed Seed for the entire system generation
     * @param planetCount Number of planets to generate
     */
    void generateSolarSystem(int systemSeed, int planetCount = 8);

    /**
     * @brief Add a single planet to the system
     * @param position World position of the planet
     * @param radius Planet radius
     * @param color Planet color
     * @param rotationSpeed Rotation speed
     * @param seed Unique seed for this planet
     * @param type Planet type (0=rocky, 1=gas, 2=ice, 3=desert)
     * @param resolution Mesh resolution (LOD)
     */
    void addPlanet(const glm::vec3& position, float radius, const glm::vec3& color, 
                   float rotationSpeed, int seed, int type = 0, int resolution = 32);

    /**
     * @brief Update all planets (rotation, etc.)
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    /**
     * @brief Render all planets with distance-based LOD
     * @param shader Planet shader to use
     * @param camera Camera for distance calculations
     * @param view View matrix
     * @param projection Projection matrix
     * @param lightPos Light position
     * @param lightColor Light color
     * @param viewPos Camera position
     */
    void render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                const glm::mat4& projection, const glm::vec3& lightPos, 
                const glm::vec3& lightColor, const glm::vec3& viewPos);

    /**
     * @brief Get the number of planets in the system
     * @return size_t Number of planets
     */
    size_t getPlanetCount() const { return planets_.size(); }

    /**
     * @brief Get planet instance by index
     * @param index Planet index
     * @return PlanetInstance* Pointer to planet instance or nullptr
     */
    PlanetInstance* getPlanet(size_t index);

    /**
     * @brief Clear all planets
     */
    void clear();

    /**
     * @brief Set maximum render distance for planets
     * @param distance Maximum distance to render planets
     */
    void setMaxRenderDistance(float distance) { maxRenderDistance_ = distance; }

    /**
     * @brief Get maximum render distance
     * @return float Maximum render distance
     */
    float getMaxRenderDistance() const { return maxRenderDistance_; }

private:
    /**
     * @brief Calculate appropriate LOD resolution based on distance
     * @param distance Distance from camera to planet
     * @param planetRadius Planet radius
     * @return int Appropriate mesh resolution
     */
    int calculateLOD(float distance, float planetRadius) const;

    /**
     * @brief Generate random planet properties based on seed
     * @param seed Planet seed
     * @param distance Distance from system center
     * @return Tuple of (radius, color, rotationSpeed, planetType)
     */
    std::tuple<float, glm::vec3, float, int> generatePlanetProperties(int seed, float distance) const;

    /**
     * @brief Generate moons for a planet
     * @param planet Planet instance to add moons to
     * @param seed Random seed for moon generation
     */
    void generateMoonsForPlanet(PlanetInstance& planet, int seed);

private:
    std::vector<std::unique_ptr<PlanetInstance>> planets_;
    Noise* noise_;
    float maxRenderDistance_;
    
    // LOD settings
    int highLOD_;    // Close planets
    int mediumLOD_;  // Medium distance planets
    int lowLOD_;     // Far planets
    
    float lodDistance1_; // Distance threshold for high->medium LOD
    float lodDistance2_; // Distance threshold for medium->low LOD
};
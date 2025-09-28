#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Sun;
class PlanetManager;
class AsteroidBelt;
class PlanetaryRings;
class ParticleSystem;
class Noise;
class Shader;
class Camera;
class Geometry;

/**
 * @brief Manages the entire solar system including Sun, planets, and their interactions
 */
class SolarSystemManager {
public:
    SolarSystemManager();
    ~SolarSystemManager();
    
    // Non-copyable and non-movable
    SolarSystemManager(const SolarSystemManager&) = delete;
    SolarSystemManager& operator=(const SolarSystemManager&) = delete;
    SolarSystemManager(SolarSystemManager&&) = delete;
    SolarSystemManager& operator=(SolarSystemManager&&) = delete;
    
    /**
     * @brief Initialize the solar system manager
     * @param noise Noise generator for procedural generation
     */
    void initialize(Noise* noise);
    
    /**
     * @brief Generate a complete solar system
     * @param systemSeed Seed for procedural generation
     * @param planetCount Number of planets to generate
     */
    void generateSolarSystem(int systemSeed, int planetCount = 8);
    
    /**
     * @brief Update all celestial bodies
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
    /**
     * @brief Render the entire solar system
     * @param planetShader Shader for rendering planets
     * @param sunShader Shader for rendering the sun
     * @param asteroidShader Shader for rendering asteroids
     * @param ringShader Shader for rendering planetary rings
     * @param particleShader Shader for rendering particle systems
     * @param camera Camera for rendering
     * @param view View matrix
     * @param projection Projection matrix
     * @param viewPos Camera position
     */
    void render(Shader* planetShader, Shader* sunShader, Shader* asteroidShader, 
                Shader* ringShader, Shader* particleShader, const Camera* camera, 
                const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);
    
    /**
     * @brief Get the sun's position (light source)
     */
    glm::vec3 getSunPosition() const;
    
    /**
     * @brief Get the sun's light color
     */
    glm::vec3 getSunLightColor() const;
    
    /**
     * @brief Get the planet manager for direct access
     */
    PlanetManager* getPlanetManager() const { return planetManager_.get(); }
    
    /**
     * @brief Get the sun for direct access
     */
    Sun* getSun() const { return sun_.get(); }
    
    /**
     * @brief Clear all celestial bodies
     */
    void clear();
    
    /**
     * @brief Set solar system scale factor
     * @param scale Scale multiplier for distances and sizes
     */
    void setScale(float scale) { systemScale_ = scale; }
    
    /**
     * @brief Get the current time scale
     * @return Current time scale multiplier
     */
    float getTimeScale() const { return timeScale_; }
    
    /**
     * @brief Set time scale for orbital speeds
     * @param timeScale Time multiplier for orbital motion
     */
    void setTimeScale(float timeScale) { timeScale_ = timeScale; }
    
    /**
     * @brief Get the current seed used for generation
     * @return Current system seed
     */
    int getSeed() const { return currentSeed_; }
    
    /**
     * @brief Get the system scale factor
     * @return Current system scale
     */
    float getSystemScale() const { return systemScale_; }
    
    /**
     * @brief Set the system scale factor
     * @param scale New scale factor
     */
    void setSystemScale(float scale) { systemScale_ = scale; }
    
    /**
     * @brief Check if asteroid belts are visible
     * @return True if asteroid belts are visible
     */
    bool getAsteroidBeltsVisible() const { return asteroidsVisible_; }
    
    /**
     * @brief Set asteroid belt visibility
     * @param visible Visibility state
     */
    void setAsteroidBeltsVisible(bool visible);
    
    /**
     * @brief Check if planetary rings are visible
     * @return True if planetary rings are visible
     */
    bool getPlanetaryRingsVisible() const { return ringsVisible_; }
    
    /**
     * @brief Set planetary rings visibility
     * @param visible Whether planetary rings should be visible
     */
    void setPlanetaryRingsVisible(bool visible);
    
    /**
     * @brief Get asteroid belt density
     * @return Current asteroid density multiplier
     */
    float getAsteroidDensity() const { return asteroidDensity_; }
    
    /**
     * @brief Set asteroid belt density
     * @param density Density multiplier (0.1 to 2.0)
     */
    void setAsteroidDensity(float density);
    
    /**
     * @brief Get planetary rings density
     * @return Current ring density multiplier
     */
    float getRingDensity() const { return ringDensity_; }
    
    /**
     * @brief Set planetary rings density
     * @param density Density multiplier (0.1 to 3.0)
     */
    void setRingDensity(float density);
    
    /**
     * @brief Check if particle systems are visible
     * @return True if particle systems are visible
     */
    bool getParticleSystemsVisible() const { return particlesVisible_; }
    
    /**
     * @brief Set particle systems visibility
     * @param visible Whether particle systems should be visible
     */
    void setParticleSystemsVisible(bool visible);
    
    /**
     * @brief Get particle emission rate
     * @return Current particle emission rate multiplier
     */
    float getParticleEmissionRate() const { return particleEmissionRate_; }
    
    /**
     * @brief Set particle emission rate
     * @param rate Emission rate multiplier (0.1 to 5.0)
     */
    void setParticleEmissionRate(float rate);

private:
    std::unique_ptr<Sun> sun_;
    std::unique_ptr<PlanetManager> planetManager_;
    std::vector<std::unique_ptr<AsteroidBelt>> asteroidBelts_;
    std::vector<std::unique_ptr<PlanetaryRings>> planetaryRings_;
    std::vector<std::unique_ptr<ParticleSystem>> particleSystems_;
    std::unique_ptr<Geometry> asteroidGeometry_;
    Noise* noise_;
    
    int currentSeed_;        // Current system seed
    float systemScale_;     // Scale factor for the entire system
    float timeScale_;       // Time scale for orbital motion
    bool initialized_;
    bool asteroidsVisible_;
    bool ringsVisible_;
    bool particlesVisible_;
    float asteroidDensity_;
    float ringDensity_;
    float particleEmissionRate_;
    
    /**
     * @brief Setup the sun for the solar system
     * @param systemSeed Seed for sun generation
     */
    void setupSun(int systemSeed);
    
    /**
     * @brief Generate asteroid belts for the solar system
     * @param systemSeed Seed for generation
     */
    void generateAsteroidBelts(int systemSeed);
    
    /**
     * @brief Generate planetary rings for gas giants
     * @param systemSeed Seed for generation
     */
    void generatePlanetaryRings(int systemSeed);
    
    /**
     * @brief Generate particle systems for stellar phenomena
     * @param systemSeed Seed for generation
     */
    void generateParticleSystems(int systemSeed);
};
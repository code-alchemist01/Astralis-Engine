#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class Camera;

enum class ParticleType {
    SOLAR_FLARE,
    COSMIC_DUST,
    STELLAR_WIND,
    CORONA_PARTICLES
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 color;
    float size;
    float life;
    float maxLife;
    float alpha;
    float temperature;
    ParticleType type;
    
    // Solar flare specific properties
    float intensity;
    float magneticField;
    
    // Cosmic dust specific properties
    float density;
    float reflectivity;
};

/**
 * @brief Advanced particle system for stellar phenomena
 */
class ParticleSystem {
public:
    ParticleSystem(const glm::vec3& origin, ParticleType type, int maxParticles = 1000);
    ~ParticleSystem();

    // Non-copyable, non-movable
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) = delete;
    ParticleSystem& operator=(ParticleSystem&&) = delete;

    void initialize();
    void update(float deltaTime);
    void render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                const glm::mat4& projection, const glm::vec3& lightPos, 
                const glm::vec3& lightColor, const glm::vec3& viewPos);

    // Particle emission
    void emitParticles(int count, const glm::vec3& emissionPoint, 
                      const glm::vec3& direction, float spread = 0.1f);
    void emitSolarFlare(const glm::vec3& sunPosition, const glm::vec3& direction, 
                       float intensity, float magneticStrength);
    void emitCosmicDust(const glm::vec3& center, float radius, int count);
    void emitStellarWind(const glm::vec3& sunPosition, float windSpeed, float density);

    // Getters
    const glm::vec3& getOrigin() const { return origin_; }
    ParticleType getType() const { return type_; }
    int getActiveParticleCount() const { return activeParticles_; }
    bool isActive() const { return active_; }

    // Setters
    void setOrigin(const glm::vec3& origin) { origin_ = origin; }
    void setActive(bool active) { active_ = active; }
    void setEmissionRate(float rate) { emissionRate_ = rate; }
    void setGravityStrength(float strength) { gravityStrength_ = strength; }
    void setMagneticFieldStrength(float strength) { magneticFieldStrength_ = strength; }

    // Physics parameters
    void setPhysicsParameters(float gravity, float magneticField, float solarWind);

private:
    void updateParticlePhysics(Particle& particle, float deltaTime);
    void updateSolarFlareParticle(Particle& particle, float deltaTime);
    void updateCosmicDustParticle(Particle& particle, float deltaTime);
    void updateStellarWindParticle(Particle& particle, float deltaTime);
    void updateCoronaParticle(Particle& particle, float deltaTime);
    
    void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                      const glm::vec3& color, float size, float life);
    void removeDeadParticles();
    void setupRenderingBuffers();
    void cleanupBuffers();
    
    // Calculate particle color based on temperature
    glm::vec3 calculateTemperatureColor(float temperature);
    
    // Apply magnetic field effects
    void applyMagneticForce(Particle& particle, float deltaTime);
    
    // Apply solar wind effects
    void applySolarWindForce(Particle& particle, float deltaTime);

    glm::vec3 origin_;
    ParticleType type_;
    int maxParticles_;
    int activeParticles_;
    bool active_;
    
    // Emission parameters
    float emissionRate_;
    float emissionTimer_;
    
    // Physics parameters
    float gravityStrength_;
    float magneticFieldStrength_;
    float solarWindStrength_;
    float temperatureDecay_;
    
    // Rendering parameters
    float maxRenderDistance_;
    bool useTemperatureColoring_;
    bool useBloom_;
    
    std::vector<Particle> particles_;
    
    // OpenGL buffers for instanced rendering
    unsigned int VAO_;
    unsigned int VBO_;
    unsigned int instanceVBO_;
    bool buffersInitialized_;
};
#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class Camera;

struct RingParticle {
    glm::vec3 position;
    float orbitRadius;
    float orbitAngle;
    float orbitSpeed;
    float size;
    glm::vec3 color;
    float alpha;
};

class PlanetaryRings {
public:
    PlanetaryRings(const glm::vec3& planetPosition, float planetRadius, 
                   float innerRadius, float outerRadius, int particleCount, int seed = 0);
    ~PlanetaryRings();

    // Non-copyable, non-movable
    PlanetaryRings(const PlanetaryRings&) = delete;
    PlanetaryRings& operator=(const PlanetaryRings&) = delete;
    PlanetaryRings(PlanetaryRings&&) = delete;
    PlanetaryRings& operator=(PlanetaryRings&&) = delete;

    void initialize();
    void update(float deltaTime);
    void render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                const glm::mat4& projection, const glm::vec3& lightPos, 
                const glm::vec3& lightColor, const glm::vec3& viewPos);

    // Getters
    const glm::vec3& getPlanetPosition() const { return planetPosition_; }
    float getInnerRadius() const { return innerRadius_; }
    float getOuterRadius() const { return outerRadius_; }
    int getParticleCount() const { return particles_.size(); }
    bool isVisible() const { return visible_; }

    // Setters
    void setPlanetPosition(const glm::vec3& position) { planetPosition_ = position; }
    void setVisible(bool visible) { visible_ = visible; }
    void setDensity(float density);
    void setOrbitSpeed(float speed) { orbitSpeedMultiplier_ = speed; }
    void setOpacity(float opacity) { opacityMultiplier_ = opacity; }

private:
    void generateRingParticles();
    void updateParticlePositions(float deltaTime);
    void setupRenderingBuffers();
    void cleanupBuffers();

    glm::vec3 planetPosition_;
    float planetRadius_;
    float innerRadius_;
    float outerRadius_;
    int particleCount_;
    int seed_;
    bool visible_;
    float orbitSpeedMultiplier_;
    float opacityMultiplier_;
    float maxRenderDistance_;

    std::vector<RingParticle> particles_;
    
    // OpenGL buffers for instanced rendering
    unsigned int VAO_;
    unsigned int VBO_;
    unsigned int instanceVBO_;
    bool buffersInitialized_;
};
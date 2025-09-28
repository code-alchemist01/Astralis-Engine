#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class Camera;
class Geometry;

struct Asteroid {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 rotationSpeed;
    float scale;
    float orbitRadius;
    float orbitAngle;
    float orbitSpeed;
    glm::vec3 color;
};

class AsteroidBelt {
public:
    AsteroidBelt(float innerRadius, float outerRadius, int asteroidCount, int seed = 0);
    ~AsteroidBelt();

    // Non-copyable, non-movable
    AsteroidBelt(const AsteroidBelt&) = delete;
    AsteroidBelt& operator=(const AsteroidBelt&) = delete;
    AsteroidBelt(AsteroidBelt&&) = delete;
    AsteroidBelt& operator=(AsteroidBelt&&) = delete;

    void initialize(Geometry* asteroidGeometry);
    void update(float deltaTime);
    void render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                const glm::mat4& projection, const glm::vec3& lightPos, 
                const glm::vec3& lightColor, const glm::vec3& viewPos);

    // Getters
    float getInnerRadius() const { return innerRadius_; }
    float getOuterRadius() const { return outerRadius_; }
    int getAsteroidCount() const { return asteroids_.size(); }
    bool isVisible() const { return visible_; }

    // Setters
    void setVisible(bool visible) { visible_ = visible; }
    void setDensity(float density);
    void setOrbitSpeed(float speed) { orbitSpeedMultiplier_ = speed; }

private:
    void generateAsteroids();
    void updateAsteroidPositions(float deltaTime);

    float innerRadius_;
    float outerRadius_;
    int asteroidCount_;
    int seed_;
    bool visible_;
    float orbitSpeedMultiplier_;
    float maxRenderDistance_;

    std::vector<Asteroid> asteroids_;
    Geometry* asteroidGeometry_;
};
#include "AsteroidBelt.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include <random>
#include <algorithm>
#include <spdlog/spdlog.h>

AsteroidBelt::AsteroidBelt(float innerRadius, float outerRadius, int asteroidCount, int seed)
    : innerRadius_(innerRadius)
    , outerRadius_(outerRadius)
    , asteroidCount_(asteroidCount)
    , seed_(seed)
    , visible_(true)
    , orbitSpeedMultiplier_(1.0f)
    , maxRenderDistance_(5000.0f)
    , asteroidGeometry_(nullptr)
{
    generateAsteroids();
    spdlog::info("Created asteroid belt: inner={:.1f}, outer={:.1f}, count={}", 
                 innerRadius_, outerRadius_, asteroidCount_);
}

AsteroidBelt::~AsteroidBelt() = default;

void AsteroidBelt::initialize(Geometry* asteroidGeometry) {
    asteroidGeometry_ = asteroidGeometry;
}

void AsteroidBelt::generateAsteroids() {
    asteroids_.clear();
    asteroids_.reserve(asteroidCount_);

    std::mt19937 rng(seed_);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> radiusDist(innerRadius_, outerRadius_);
    std::uniform_real_distribution<float> heightDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> scaleDist(0.1f, 0.8f);
    std::uniform_real_distribution<float> rotSpeedDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> orbitSpeedDist(0.1f, 0.5f);
    std::uniform_real_distribution<float> colorVariation(0.3f, 0.8f);

    for (int i = 0; i < asteroidCount_; ++i) {
        Asteroid asteroid;
        
        // Orbital parameters
        asteroid.orbitRadius = radiusDist(rng);
        asteroid.orbitAngle = angleDist(rng);
        asteroid.orbitSpeed = orbitSpeedDist(rng) / asteroid.orbitRadius; // Slower for outer asteroids
        
        // Position (will be updated in updateAsteroidPositions)
        float height = heightDist(rng);
        asteroid.position = glm::vec3(
            asteroid.orbitRadius * cos(asteroid.orbitAngle),
            height,
            asteroid.orbitRadius * sin(asteroid.orbitAngle)
        );
        
        // Rotation
        asteroid.rotation = glm::vec3(angleDist(rng), angleDist(rng), angleDist(rng));
        asteroid.rotationSpeed = glm::vec3(rotSpeedDist(rng), rotSpeedDist(rng), rotSpeedDist(rng));
        
        // Scale
        asteroid.scale = scaleDist(rng);
        
        // Color (grayish-brown rocky appearance)
        float baseGray = colorVariation(rng);
        asteroid.color = glm::vec3(
            baseGray * 0.8f,
            baseGray * 0.7f,
            baseGray * 0.6f
        );
        
        asteroids_.push_back(asteroid);
    }
}

void AsteroidBelt::update(float deltaTime) {
    if (!visible_) return;
    
    updateAsteroidPositions(deltaTime);
}

void AsteroidBelt::updateAsteroidPositions(float deltaTime) {
    for (auto& asteroid : asteroids_) {
        // Update orbital position
        asteroid.orbitAngle += asteroid.orbitSpeed * orbitSpeedMultiplier_ * deltaTime;
        
        // Keep angle in range [0, 2π]
        if (asteroid.orbitAngle > 2.0f * 3.14159f) {
            asteroid.orbitAngle -= 2.0f * 3.14159f;
        }
        
        // Update position based on orbital mechanics
        asteroid.position.x = asteroid.orbitRadius * cos(asteroid.orbitAngle);
        asteroid.position.z = asteroid.orbitRadius * sin(asteroid.orbitAngle);
        // Y position stays relatively constant (slight orbital inclination)
        
        // Update rotation
        asteroid.rotation += asteroid.rotationSpeed * deltaTime;
    }
}

void AsteroidBelt::render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                         const glm::mat4& projection, const glm::vec3& lightPos, 
                         const glm::vec3& lightColor, const glm::vec3& viewPos) {
    if (!visible_ || !shader || !camera || !asteroidGeometry_ || !asteroidGeometry_->isValid()) {
        return;
    }

    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("viewPos", viewPos);

    glm::vec3 cameraPos = camera->getPosition();
    int asteroidsRendered = 0;

    for (const auto& asteroid : asteroids_) {
        float distance = glm::length(asteroid.position - cameraPos);
        
        // Skip asteroids that are too far away
        if (distance > maxRenderDistance_) {
            continue;
        }

        // Create model matrix
        glm::mat4 model = glm::translate(glm::mat4(1.0f), asteroid.position);
        model = glm::rotate(model, asteroid.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(asteroid.scale));

        // Set uniforms
        shader->setMat4("model", model);
        shader->setVec3("planetColor", asteroid.color);
        shader->setFloat("planetSeed", static_cast<float>(seed_ + (&asteroid - &asteroids_[0])));
        shader->setInt("planetType", 0); // Rocky type for asteroids

        // Render asteroid
        asteroidGeometry_->draw();
        asteroidsRendered++;
    }

    shader->unuse();

    // Log rendering stats occasionally
    static int frameCount = 0;
    if (++frameCount % 300 == 0) { // Every 5 seconds at 60 FPS
        spdlog::debug("Rendered {}/{} asteroids in belt", asteroidsRendered, asteroids_.size());
    }
}

void AsteroidBelt::setDensity(float density) {
    // Clamp density between 0.1 and 2.0
    density = std::clamp(density, 0.1f, 2.0f);
    
    int newCount = static_cast<int>(asteroidCount_ * density);
    if (newCount != static_cast<int>(asteroids_.size())) {
        asteroidCount_ = newCount;
        generateAsteroids();
        spdlog::info("Updated asteroid belt density: new count = {}", asteroidCount_);
    }
}
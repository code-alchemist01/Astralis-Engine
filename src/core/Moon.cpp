#include "Moon.hpp"
#include "Planet.hpp"
#include "Geometry.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Moon::Moon(float radius, float orbitRadius, float orbitSpeed, const glm::vec3& color, int resolution)
    : radius_(radius)
    , orbitRadius_(orbitRadius)
    , orbitSpeed_(orbitSpeed)
    , color_(color)
    , position_(glm::vec3(0.0f))
    , currentOrbitAngle_(0.0f)
    , orbitInclination_(0.0f)
    , rotationSpeed_(2.0f)
    , currentRotation_(0.0f)
{
    // Create moon geometry using Planet class (sphere)
    planet_ = std::make_unique<Planet>(radius_, resolution, nullptr);
    planet_->generate();
    
    // Set random orbital inclination (small angle for realistic moon orbits)
    orbitInclination_ = ((rand() % 100) / 100.0f - 0.5f) * 0.2f; // ±0.1 radians (~±6 degrees)
}

void Moon::update(float deltaTime, const glm::vec3& planetPosition) {
    // Update orbital angle
    currentOrbitAngle_ += orbitSpeed_ * deltaTime;
    if (currentOrbitAngle_ > 2.0f * M_PI) {
        currentOrbitAngle_ -= 2.0f * M_PI;
    }
    
    // Update rotation
    currentRotation_ += rotationSpeed_ * deltaTime;
    if (currentRotation_ > 2.0f * M_PI) {
        currentRotation_ -= 2.0f * M_PI;
    }
    
    // Calculate orbital position relative to planet
    float x = orbitRadius_ * cos(currentOrbitAngle_);
    float z = orbitRadius_ * sin(currentOrbitAngle_);
    float y = orbitRadius_ * sin(orbitInclination_) * sin(currentOrbitAngle_);
    
    // Set world position relative to planet
    position_ = planetPosition + glm::vec3(x, y, z);
}

void Moon::render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                  const glm::mat4& projection, const glm::vec3& lightPos, 
                  const glm::vec3& lightColor, const glm::vec3& viewPos) {
    if (!planet_ || !shader) {
        return;
    }
    
    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position_);
    model = glm::rotate(model, currentRotation_, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Set shader uniforms
    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("viewPos", viewPos);
    shader->setVec3("planetColor", color_);
    
    // Render moon geometry
    if (planet_->getGeometry() && planet_->getGeometry()->isValid()) {
        planet_->getGeometry()->draw();
    }
}
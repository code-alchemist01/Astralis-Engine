#include "Sun.hpp"
#include "Geometry.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float PI = 3.14159265359f;

Sun::Sun(const glm::vec3& position, float radius, const glm::vec3& color, 
         float temperature, float intensity)
    : position_(position)
    , radius_(radius)
    , color_(color)
    , temperature_(temperature)
    , intensity_(intensity)
    , currentRotation_(0.0f)
    , rotationSpeed_(10.0f)  // Slow rotation for the sun
    , pulsePhase_(0.0f)
    , pulseIntensity_(0.1f)
    , solarFlareIntensity_(0.0f)
    , solarFlarePhase_(0.0f)
    , baseIntensity_(intensity)
    , currentLightIntensity_(intensity)
    , geometry_(nullptr)
{
}

void Sun::initialize(int resolution) {
    // Create sphere geometry for the sun
    geometry_ = std::make_unique<Geometry>();
    geometry_->createSphere(radius_, resolution, resolution);
    
    // Calculate color based on temperature
    color_ = calculateTemperatureColor(temperature_);
}

void Sun::initialize(const glm::vec3& position, float radius, float temperature, float intensity, int resolution) {
    // Set sun properties
    position_ = position;
    radius_ = radius;
    temperature_ = temperature;
    intensity_ = intensity;
    
    // Create sphere geometry for the sun
    geometry_ = std::make_unique<Geometry>();
    geometry_->createSphere(radius_, resolution, resolution);
    
    // Calculate color based on temperature
    color_ = calculateTemperatureColor(temperature_);
}

void Sun::update(float deltaTime) {
    // Update rotation
    currentRotation_ += rotationSpeed_ * deltaTime;
    if (currentRotation_ > 360.0f) {
        currentRotation_ -= 360.0f;
    }
    
    // Update pulsing effect
    pulsePhase_ += deltaTime * 2.0f; // Pulse frequency
    if (pulsePhase_ > 2.0f * PI) {
        pulsePhase_ -= 2.0f * PI;
    }
    
    // Update solar flare activity
    solarFlarePhase_ += deltaTime * 0.5f; // Slower flare cycle
    if (solarFlarePhase_ > 2.0f * PI) {
        solarFlarePhase_ -= 2.0f * PI;
    }
    
    // Calculate dynamic solar flare intensity
    float flareBase = std::sin(solarFlarePhase_) * 0.5f + 0.5f; // 0 to 1
    float flareNoise = std::sin(solarFlarePhase_ * 3.7f) * 0.3f; // Add some irregularity
    solarFlareIntensity_ = flareBase + flareNoise;
    solarFlareIntensity_ = std::max(0.0f, std::min(1.0f, solarFlareIntensity_)); // Clamp to [0,1]
    
    // Calculate current light intensity with solar activity
    float activityMultiplier = 1.0f + solarFlareIntensity_ * 0.4f; // Up to 40% intensity boost
    float pulseMultiplier = 1.0f + pulseIntensity_ * std::sin(pulsePhase_);
    currentLightIntensity_ = baseIntensity_ * activityMultiplier * pulseMultiplier;
}

void Sun::render(Shader* shader, const Camera* camera, 
                 const glm::mat4& view, const glm::mat4& projection) {
    if (!geometry_ || !shader) return;
    
    shader->use();
    
    // Calculate model matrix with rotation and pulsing scale
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(currentRotation_), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Add subtle pulsing effect
    float pulseScale = 1.0f + pulseIntensity_ * std::sin(pulsePhase_);
    model = glm::scale(model, glm::vec3(pulseScale));
    
    // Set shader uniforms
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    // Sun-specific uniforms
    shader->setVec3("sunColor", color_);
    shader->setFloat("sunIntensity", intensity_);
    shader->setFloat("sunTemperature", temperature_);
    shader->setFloat("pulsePhase", pulsePhase_);
    shader->setFloat("solarFlareIntensity", solarFlareIntensity_);
    shader->setFloat("currentLightIntensity", currentLightIntensity_);
    
    // Light properties for self-illumination
    shader->setVec3("lightPos", position_);
    shader->setVec3("lightColor", color_ * intensity_);
    shader->setVec3("viewPos", camera->getPosition());
    
    // Render the sun geometry
    geometry_->bind();
    geometry_->draw();
    geometry_->unbind();
}

Sun::LightProperties Sun::getLightProperties() const {
    LightProperties props;
    props.position = position_;
    props.color = color_;
    props.intensity = currentLightIntensity_; // Use dynamic intensity
    return props;
}

glm::vec3 Sun::calculateTemperatureColor(float temperature) const {
    // Simplified blackbody radiation color calculation
    // Based on temperature in Kelvin
    
    glm::vec3 color;
    
    if (temperature < 3500.0f) {
        // Red dwarf stars
        color = glm::vec3(1.0f, 0.3f, 0.1f);
    }
    else if (temperature < 5000.0f) {
        // Orange stars
        color = glm::vec3(1.0f, 0.6f, 0.2f);
    }
    else if (temperature < 6000.0f) {
        // Yellow stars (like our Sun)
        color = glm::vec3(1.0f, 0.9f, 0.7f);
    }
    else if (temperature < 7500.0f) {
        // White stars
        color = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    else if (temperature < 10000.0f) {
        // Blue-white stars
        color = glm::vec3(0.8f, 0.9f, 1.0f);
    }
    else {
        // Blue giants
        color = glm::vec3(0.6f, 0.7f, 1.0f);
    }
    
    return color;
}
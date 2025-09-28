#include "SolarSystemManager.hpp"
#include "Sun.hpp"
#include "Planet.hpp"
#include "PlanetManager.hpp"
#include "AsteroidBelt.hpp"
#include "PlanetaryRings.hpp"
#include "Geometry.hpp"
#include "Noise.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <spdlog/spdlog.h>
#include <random>

SolarSystemManager::SolarSystemManager()
    : sun_(nullptr)
    , planetManager_(nullptr)
    , asteroidGeometry_(nullptr)
    , noise_(nullptr)
    , systemScale_(1.0f)
    , timeScale_(1.0f)
    , initialized_(false)
    , asteroidsVisible_(true)
    , ringsVisible_(true)
{
}

SolarSystemManager::~SolarSystemManager() = default;

void SolarSystemManager::initialize(Noise* noise) {
    if (!noise) {
        spdlog::error("Cannot initialize SolarSystemManager: noise generator is null");
        return;
    }
    
    noise_ = noise;
    
    // Create sun
    sun_ = std::make_unique<Sun>();
    
    // Create planet manager
    planetManager_ = std::make_unique<PlanetManager>();
    planetManager_->initialize(noise);
    
    // Create asteroid geometry (simple sphere for asteroids)
    asteroidGeometry_ = std::make_unique<Geometry>();
    asteroidGeometry_->createSphere(1.0f, 8, 6); // Low-poly sphere for performance
    
    initialized_ = true;
    spdlog::info("SolarSystemManager initialized successfully");
}

void SolarSystemManager::generateSolarSystem(int systemSeed, int planetCount) {
    if (!initialized_) {
        spdlog::error("Cannot generate solar system: manager not initialized");
        return;
    }
    
    spdlog::info("Generating solar system with seed {} and {} planets", systemSeed, planetCount);
    
    // Clear existing system
    clear();
    
    // Setup sun
    setupSun(systemSeed);
    
    // Generate planets
    planetManager_->generateSolarSystem(systemSeed, planetCount);
    
    // Generate asteroid belts
    generateAsteroidBelts(systemSeed);
    
    // Generate planetary rings
    generatePlanetaryRings(systemSeed);
    
    spdlog::info("Solar system generated successfully");
}

void SolarSystemManager::update(float deltaTime) {
    if (!initialized_) {
        return;
    }
    
    float scaledDeltaTime = deltaTime * timeScale_;
    
    // Update sun
    if (sun_) {
        sun_->update(scaledDeltaTime);
    }
    
    // Update planets
    if (planetManager_) {
        planetManager_->update(scaledDeltaTime);
    }
    
    // Update asteroid belts
    for (auto& belt : asteroidBelts_) {
        if (belt) {
            belt->update(scaledDeltaTime);
        }
    }
    
    // Update planetary rings
    for (auto& rings : planetaryRings_) {
        if (rings) {
            rings->update(scaledDeltaTime);
        }
    }
}

void SolarSystemManager::render(Shader* planetShader, Shader* sunShader, Shader* asteroidShader, 
                               Shader* ringShader, const Camera* camera, const glm::mat4& view, 
                               const glm::mat4& projection, const glm::vec3& viewPos) {
    if (!initialized_ || !camera) {
        return;
    }
    
    // Get sun properties for lighting
    glm::vec3 sunPos = getSunPosition();
    glm::vec3 sunColor = getSunLightColor();
    float lightIntensity = sun_ ? sun_->getCurrentLightIntensity() : 1.0f;
    
    // Render planets first (they need sun lighting)
    if (planetManager_ && planetShader) {
        planetManager_->render(planetShader, camera, view, projection, 
                              sunPos, sunColor, viewPos, lightIntensity);
    }
    
    // Render asteroid belts
    if (asteroidsVisible_ && asteroidShader) {
        for (auto& belt : asteroidBelts_) {
            if (belt && belt->isVisible()) {
                belt->render(asteroidShader, camera, view, projection, 
                           sunPos, sunColor, viewPos);
            }
        }
    }
    
    // Render planetary rings
    if (ringsVisible_ && ringShader) {
        for (auto& rings : planetaryRings_) {
            if (rings && rings->isVisible()) {
                rings->render(ringShader, camera, view, projection, 
                            sunPos, sunColor, viewPos);
            }
        }
    }
    
    // Render sun last (it's self-illuminated)
    if (sun_ && sunShader) {
        sun_->render(sunShader, camera, view, projection);
    }
}

glm::vec3 SolarSystemManager::getSunPosition() const {
    if (sun_) {
        return sun_->getPosition();
    }
    return glm::vec3(0.0f);
}

glm::vec3 SolarSystemManager::getSunLightColor() const {
    if (sun_) {
        return sun_->getLightProperties().color;
    }
    return glm::vec3(1.0f, 1.0f, 0.9f); // Default warm white
}

void SolarSystemManager::clear() {
    if (planetManager_) {
        planetManager_->clear();
    }
    
    // Clear asteroid belts
    asteroidBelts_.clear();
    
    // Clear planetary rings
    planetaryRings_.clear();
    
    spdlog::debug("Solar system cleared");
}

void SolarSystemManager::setupSun(int systemSeed) {
    if (!sun_) {
        spdlog::error("Cannot setup sun: sun object is null");
        return;
    }
    
    std::mt19937 rng(systemSeed);
    std::uniform_real_distribution<float> sizeDist(12.0f, 16.0f); // Much larger sun
    std::uniform_real_distribution<float> tempDist(5500.0f, 6000.0f); // Keep it in yellow range
    std::uniform_real_distribution<float> colorVariation(0.9f, 1.1f);
    
    float sunSize = sizeDist(rng);
    float temperature = tempDist(rng);
    
    // Color based on temperature - make it more yellow/orange
    glm::vec3 sunColor;
    if (temperature < 5700.0f) {
        sunColor = glm::vec3(1.0f, 0.8f, 0.4f) * colorVariation(rng); // Orange-yellow
    } else if (temperature < 5900.0f) {
        sunColor = glm::vec3(1.0f, 0.9f, 0.6f) * colorVariation(rng); // Yellow
    } else {
        sunColor = glm::vec3(1.0f, 0.95f, 0.8f) * colorVariation(rng); // Warm yellow
    }
    
    sun_->setRadius(sunSize);
    sun_->setColor(sunColor);
    sun_->setTemperature(temperature);
    
    // Initialize the sun geometry
    sun_->initialize(64); // High resolution sphere for the sun
    
    spdlog::info("Sun setup complete: size={:.2f}, temp={:.0f}K", sunSize, temperature);
}

void SolarSystemManager::generateAsteroidBelts(int systemSeed) {
    asteroidBelts_.clear();
    
    std::mt19937 rng(systemSeed + 1000); // Different seed for asteroids
    std::uniform_int_distribution<int> beltCountDist(1, 3);
    std::uniform_real_distribution<float> innerRadiusDist(40.0f, 80.0f);
    std::uniform_real_distribution<float> widthDist(15.0f, 30.0f);
    std::uniform_int_distribution<int> asteroidCountDist(200, 800);
    
    int beltCount = beltCountDist(rng);
    
    for (int i = 0; i < beltCount; ++i) {
        float innerRadius = innerRadiusDist(rng) + i * 50.0f; // Space belts apart
        float outerRadius = innerRadius + widthDist(rng);
        int asteroidCount = asteroidCountDist(rng);
        
        auto belt = std::make_unique<AsteroidBelt>(innerRadius, outerRadius, asteroidCount, systemSeed + i);
        belt->initialize(asteroidGeometry_.get());
        asteroidBelts_.push_back(std::move(belt));
    }
    
    spdlog::info("Generated {} asteroid belts", beltCount);
}

void SolarSystemManager::generatePlanetaryRings(int systemSeed) {
    planetaryRings_.clear();
    
    if (!planetManager_) {
        return;
    }
    
    std::mt19937 rng(systemSeed + 2000); // Different seed for rings
    std::uniform_real_distribution<float> ringChance(0.0f, 1.0f);
    std::uniform_real_distribution<float> ringWidthDist(2.0f, 8.0f);
    std::uniform_int_distribution<int> particleCountDist(500, 2000);
    
    // Get planet information from planet manager
    // Note: This is a simplified approach - in a real implementation,
    // we'd need access to planet data from PlanetManager
    
    // For now, create rings at typical gas giant distances
    std::vector<float> gasGiantDistances = {60.0f, 95.0f, 130.0f};
    std::vector<float> gasGiantRadii = {8.0f, 12.0f, 10.0f};
    
    for (size_t i = 0; i < gasGiantDistances.size(); ++i) {
        if (ringChance(rng) > 0.4f) { // 60% chance for rings
            float planetRadius = gasGiantRadii[i];
            float innerRadius = planetRadius * 1.5f;
            float outerRadius = innerRadius + ringWidthDist(rng);
            int particleCount = particleCountDist(rng);
            
            glm::vec3 planetPosition(gasGiantDistances[i], 0.0f, 0.0f); // Simplified position
            
            auto rings = std::make_unique<PlanetaryRings>(planetPosition, planetRadius, 
                                                         innerRadius, outerRadius, 
                                                         particleCount, systemSeed + static_cast<int>(i));
            rings->initialize();
            planetaryRings_.push_back(std::move(rings));
        }
    }
    
    spdlog::info("Generated {} planetary ring systems", planetaryRings_.size());
}

void SolarSystemManager::setAsteroidBeltsVisible(bool visible) {
    asteroidsVisible_ = visible;
    for (auto& belt : asteroidBelts_) {
        if (belt) {
            belt->setVisible(visible);
        }
    }
}

void SolarSystemManager::setPlanetaryRingsVisible(bool visible) {
    ringsVisible_ = visible;
    for (auto& rings : planetaryRings_) {
        if (rings) {
            rings->setVisible(visible);
        }
    }
}

void SolarSystemManager::setAsteroidDensity(float density) {
    for (auto& belt : asteroidBelts_) {
        if (belt) {
            belt->setDensity(density);
        }
    }
}

void SolarSystemManager::setRingDensity(float density) {
    for (auto& rings : planetaryRings_) {
        if (rings) {
            rings->setDensity(density);
        }
    }
}
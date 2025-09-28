#include "PlanetManager.hpp"
#include "Planet.hpp"
#include "Moon.hpp"
#include "Noise.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include <random>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

PlanetManager::PlanetManager()
    : noise_(nullptr)
    , maxRenderDistance_(1000000000.0f)  // Increased from 1000 to 10000 for better visibility
    , highLOD_(64)
    , mediumLOD_(32)
    , lowLOD_(16)
    , lodDistance1_(100.0f)  // Increased LOD distances as well
    , lodDistance2_(500.0f)
{
}

void PlanetManager::initialize(Noise* noise) {
    noise_ = noise;
    spdlog::info("PlanetManager initialized with noise generator");
}

void PlanetManager::generateSolarSystem(int systemSeed, int planetCount) {
    clear();
    
    std::mt19937 rng(systemSeed);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> distanceDist(25.0f, 120.0f);
    std::uniform_real_distribution<float> heightDist(-10.0f, 10.0f);
    
    spdlog::info("Generating solar system with {} planets (seed: {})", planetCount, systemSeed);
    
    std::vector<std::pair<glm::vec3, float>> planetPositions; // Store positions and radii for collision detection
    
    for (int i = 0; i < planetCount; ++i) {
        glm::vec3 position;
        float radius;
        bool validPosition = false;
        int attempts = 0;
        const int maxAttempts = 50;
        
        do {
            // Generate orbital position
            float angle = angleDist(rng);
            float distance = distanceDist(rng);
            float height = heightDist(rng);
            
            position = glm::vec3(
                distance * cos(angle),
                height,
                distance * sin(angle)
            );
            
            // Generate planet properties based on distance and seed
            int planetSeed = systemSeed + i * 1000 + attempts;
            auto [tempRadius, color, rotationSpeed, planetType] = generatePlanetProperties(planetSeed, distance);
            radius = tempRadius;
            
            // Check for collisions with existing planets
            validPosition = true;
            for (const auto& [existingPos, existingRadius] : planetPositions) {
                float minDistance = radius + existingRadius + 5.0f; // 5 unit buffer
                float actualDistance = glm::length(position - existingPos);
                
                if (actualDistance < minDistance) {
                    validPosition = false;
                    break;
                }
            }
            
            attempts++;
        } while (!validPosition && attempts < maxAttempts);
        
        if (validPosition) {
            // Generate final planet properties
            int planetSeed = systemSeed + i * 1000;
            auto [finalRadius, color, rotationSpeed, planetType] = generatePlanetProperties(planetSeed, glm::length(position));
            
            addPlanet(position, finalRadius, color, rotationSpeed, planetSeed, planetType);
            planetPositions.emplace_back(position, finalRadius);
            
            spdlog::info("Generated planet {}: pos({:.1f}, {:.1f}, {:.1f}), radius={:.1f}, seed={}", 
                         i, position.x, position.y, position.z, finalRadius, planetSeed);
        } else {
            spdlog::warn("Could not find valid position for planet {} after {} attempts", i, maxAttempts);
        }
    }
    
    spdlog::info("Solar system generation complete: {} planets created", planets_.size());
}

void PlanetManager::addPlanet(const glm::vec3& position, float radius, const glm::vec3& color, 
                             float rotationSpeed, int seed, int type, int resolution) {
    if (!noise_) {
        spdlog::error("Cannot add planet: noise generator not initialized");
        return;
    }
    
    // Create planet with initial resolution
    auto planet = std::make_unique<Planet>(radius, resolution, noise_);
    
    // Set planet-specific noise parameters based on seed
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> heightDist(0.1f, 0.8f);
    std::uniform_real_distribution<float> freqDist(0.01f, 0.05f);
    std::uniform_int_distribution<int> octaveDist(3, 6);
    
    planet->setHeightScale(heightDist(rng));
    planet->setNoiseFrequency(freqDist(rng));
    planet->setNoiseOctaves(octaveDist(rng));
    planet->generate();
    
    // Create planet instance
    auto instance = std::make_unique<PlanetInstance>(
        std::move(planet), position, 1.0f, color, rotationSpeed, seed, type
    );
    
    // Setup orbital mechanics parameters
    float distance = glm::length(position);
    instance->orbitRadius = distance;
    instance->orbitCenter = glm::vec3(0.0f); // Sun at origin
    
    // Calculate orbital speed based on Kepler's laws (simplified)
    // Closer planets orbit faster
    instance->orbitSpeed = 0.5f / sqrt(distance * 0.1f + 1.0f);
    
    // Set initial orbit angle based on position
    instance->currentOrbitAngle = atan2(position.z, position.x);
    
    // Add some orbital variation based on seed
    std::mt19937 orbitalRng(seed + 12345);
    std::uniform_real_distribution<float> inclinationDist(-0.1f, 0.1f); // Small inclinations
    std::uniform_real_distribution<float> eccentricityDist(0.0f, 0.2f);  // Slight elliptical orbits
    
    instance->orbitInclination = inclinationDist(orbitalRng);
    instance->orbitEccentricity = eccentricityDist(orbitalRng);
    
    // Generate moons for this planet
    generateMoonsForPlanet(*instance, seed);
    
    planets_.push_back(std::move(instance));
    
    spdlog::info("Added planet at ({:.1f}, {:.1f}, {:.1f}) with radius {:.1f}, type {} - Total planets: {}", 
                 position.x, position.y, position.z, radius, type, planets_.size());
}

void PlanetManager::update(float deltaTime) {
    for (auto& planetInstance : planets_) {
        // Update planet rotation
        planetInstance->currentRotation += planetInstance->rotationSpeed * deltaTime;
        
        // Keep rotation in reasonable range
        if (planetInstance->currentRotation > 2.0f * 3.14159f) {
            planetInstance->currentRotation -= 2.0f * 3.14159f;
        }
        
        // Update orbital position
        planetInstance->currentOrbitAngle += planetInstance->orbitSpeed * deltaTime;
        
        // Keep orbit angle in reasonable range
        if (planetInstance->currentOrbitAngle > 2.0f * 3.14159f) {
            planetInstance->currentOrbitAngle -= 2.0f * 3.14159f;
        }
        
        // Calculate new position based on orbital mechanics
        float angle = planetInstance->currentOrbitAngle;
        float radius = planetInstance->orbitRadius;
        
        // Apply eccentricity (elliptical orbits)
        float eccentricity = planetInstance->orbitEccentricity;
        float adjustedRadius = radius * (1.0f - eccentricity * cos(angle));
        
        // Calculate position in orbital plane
        float x = adjustedRadius * cos(angle);
        float z = adjustedRadius * sin(angle);
        float y = 0.0f;
        
        // Apply orbital inclination
        float inclination = planetInstance->orbitInclination;
        if (inclination != 0.0f) {
            float newY = y * cos(inclination) - z * sin(inclination);
            float newZ = y * sin(inclination) + z * cos(inclination);
            y = newY;
            z = newZ;
        }
        
        // Set final position relative to orbit center
        planetInstance->position = planetInstance->orbitCenter + glm::vec3(x, y, z);
        
        // Update moons
        for (auto& moon : planetInstance->moons) {
            moon->update(deltaTime, planetInstance->position);
        }
    }
}

void PlanetManager::render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                          const glm::mat4& projection, const glm::vec3& lightPos, 
                          const glm::vec3& lightColor, const glm::vec3& viewPos) {
    if (!shader || !camera) {
        return;
    }
    
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("viewPos", viewPos);
    
    glm::vec3 cameraPos = camera->getPosition();
    int planetsRendered = 0;
    
    for (auto& planetInstance : planets_) {
        float distance = glm::length(planetInstance->position - cameraPos);
        
        // Skip planets that are too far away
        if (distance > maxRenderDistance_) {
            continue;
        }
        
        // Calculate appropriate LOD
        int targetLOD = calculateLOD(distance, planetInstance->planet->getRadius());
        
        // Update planet resolution if needed (expensive operation)
        if (planetInstance->planet->getResolution() != targetLOD) {
            planetInstance->planet->setResolution(targetLOD);
            planetInstance->planet->generate();
            spdlog::debug("Updated planet LOD to {} (distance: {:.1f})", targetLOD, distance);
        }
        
        // Set up model matrix with position, scale, and rotation
        glm::mat4 model = glm::translate(glm::mat4(1.0f), planetInstance->position);
        model = glm::rotate(model, planetInstance->currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(planetInstance->scale));
        
        shader->setMat4("model", model);
        shader->setVec3("planetColor", planetInstance->color);
        shader->setFloat("planetSeed", static_cast<float>(planetInstance->seed));
        shader->setInt("planetType", planetInstance->type);
        
        // Render planet if it has valid geometry
        if (planetInstance->planet->getGeometry() && planetInstance->planet->getGeometry()->isValid()) {
            planetInstance->planet->getGeometry()->draw();
            planetsRendered++;
        }
        
        // Render moons
        for (auto& moon : planetInstance->moons) {
            float moonDistance = glm::length(moon->getPosition() - cameraPos);
            if (moonDistance <= maxRenderDistance_) {
                moon->render(shader, camera, view, projection, lightPos, lightColor, viewPos);
            }
        }
    }
    
    shader->unuse();
    
    // Log rendering stats occasionally
    static int frameCount = 0;
    if (++frameCount % 60 == 0) { // Every 1 second at 60 FPS
        spdlog::info("Rendered {}/{} planets, camera pos: ({:.1f}, {:.1f}, {:.1f})", 
                     planetsRendered, planets_.size(), cameraPos.x, cameraPos.y, cameraPos.z);
    }
}



PlanetInstance* PlanetManager::getPlanet(size_t index) {
    if (index >= planets_.size()) {
        return nullptr;
    }
    return planets_[index].get();
}

void PlanetManager::clear() {
    planets_.clear();
    spdlog::info("Cleared all planets from manager");
}

int PlanetManager::calculateLOD(float distance, float planetRadius) const {
    // Adjust LOD based on both distance and planet size
    float effectiveDistance = distance / (planetRadius + 1.0f);
    
    if (effectiveDistance < lodDistance1_) {
        return highLOD_;
    } else if (effectiveDistance < lodDistance2_) {
        return mediumLOD_;
    } else {
        return lowLOD_;
    }
}

std::tuple<float, glm::vec3, float, int> PlanetManager::generatePlanetProperties(int seed, float distance) const {
    std::mt19937 rng(seed);
    
    // Determine planet type based on distance from center
    int planetType = 0; // Default to rocky
    if (distance < 50.0f) {
        // Inner system: rocky or desert planets
        std::uniform_int_distribution<int> innerTypeDist(0, 3);
        int typeChoice = innerTypeDist(rng);
        planetType = (typeChoice <= 1) ? 0 : 3; // 50% rocky, 50% desert
    } else if (distance < 150.0f) {
        // Middle system: rocky, ice, or desert
        std::uniform_int_distribution<int> midTypeDist(0, 2);
        planetType = midTypeDist(rng); // rocky, gas, or ice
    } else {
        // Outer system: gas giants and ice planets
        std::uniform_int_distribution<int> outerTypeDist(0, 1);
        planetType = (outerTypeDist(rng) == 0) ? 1 : 2; // gas or ice
    }
    
    // Planet radius based on distance and type - more realistic distribution
    float radius;
    if (distance < 50.0f) {
        // Inner planets: smaller, rocky worlds
        std::uniform_real_distribution<float> innerRadiusDist(0.8f, 2.5f);
        radius = innerRadiusDist(rng);
    } else if (distance < 100.0f) {
        // Middle system: medium-sized planets
        std::uniform_real_distribution<float> midRadiusDist(1.5f, 4.0f);
        radius = midRadiusDist(rng);
    } else {
        // Outer system: larger planets, especially gas giants
        std::uniform_real_distribution<float> outerRadiusDist(2.0f, 8.0f);
        radius = outerRadiusDist(rng);
    }
    
    // Adjust radius based on type
    if (planetType == 1) { // Gas giants are much larger
        radius *= 2.2f;
    } else if (planetType == 2) { // Ice planets are medium-large
        radius *= 1.4f;
    }
    
    // Generate planet color based on type (will be overridden by shader)
    glm::vec3 color;
    switch(planetType) {
        case 0: // Rocky
            color = glm::vec3(0.6f, 0.5f, 0.4f);
            break;
        case 1: // Gas giant
            color = glm::vec3(0.8f, 0.6f, 0.3f);
            break;
        case 2: // Ice
            color = glm::vec3(0.7f, 0.8f, 0.9f);
            break;
        case 3: // Desert
            color = glm::vec3(0.8f, 0.7f, 0.4f);
            break;
        default:
            color = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    
    // Add some variation to color
    std::uniform_real_distribution<float> colorVariation(-0.1f, 0.1f);
    color.r = std::clamp(color.r + colorVariation(rng), 0.2f, 1.0f);
    color.g = std::clamp(color.g + colorVariation(rng), 0.2f, 1.0f);
    color.b = std::clamp(color.b + colorVariation(rng), 0.2f, 1.0f);
    
    // Rotation speed (smaller planets rotate faster, gas giants slower)
    std::uniform_real_distribution<float> rotationDist(0.1f, 2.0f);
    float rotationSpeed = rotationDist(rng) / radius;
    if (planetType == 1) { // Gas giants rotate slower
        rotationSpeed *= 0.5f;
    }
    
    return std::make_tuple(radius, color, rotationSpeed, planetType);
}

void PlanetManager::generateMoonsForPlanet(PlanetInstance& planet, int seed) {
    std::mt19937 rng(seed + 54321); // Different seed offset for moons
    
    spdlog::debug("Generating moons for planet at ({:.1f}, {:.1f}, {:.1f}), type={}, scale={:.1f}", 
                  planet.position.x, planet.position.y, planet.position.z, planet.type, planet.scale);
    
    // Determine number of moons based on planet type and size
    int maxMoons = 0;
    if (planet.type == 1) { // Gas giants
        maxMoons = 4; // Gas giants can have more moons
    } else if (planet.scale > 8.0f) { // Large planets
        maxMoons = 3;
    } else if (planet.scale > 5.0f) { // Medium planets
        maxMoons = 2;
    } else { // Small planets
        maxMoons = 1;
    }
    
    std::uniform_int_distribution<int> moonCountDist(0, maxMoons);
    int moonCount = moonCountDist(rng);
    
    spdlog::debug("Planet type={}, scale={:.1f}, maxMoons={}, generated moonCount={}", 
                  planet.type, planet.scale, maxMoons, moonCount);
    
    if (moonCount == 0) {
        spdlog::debug("No moons generated for this planet");
        return; // No moons for this planet
    }
    
    // Generate moons
    for (int i = 0; i < moonCount; ++i) {
        // Moon properties
        std::uniform_real_distribution<float> radiusDist(1.0f, planet.scale * 0.3f); // Moon size relative to planet
        std::uniform_real_distribution<float> orbitDist(planet.scale * 2.0f, planet.scale * 6.0f); // Orbit distance
        std::uniform_real_distribution<float> speedDist(0.5f, 2.0f); // Orbital speed
        std::uniform_real_distribution<float> colorVariation(0.6f, 1.0f);
        
        float moonRadius = radiusDist(rng);
        float orbitRadius = orbitDist(rng);
        float orbitSpeed = speedDist(rng);
        
        // Moon color (grayish with some variation)
        glm::vec3 moonColor = glm::vec3(
            colorVariation(rng) * 0.8f,
            colorVariation(rng) * 0.8f,
            colorVariation(rng) * 0.8f
        );
        
        // Create moon
        auto moon = std::make_unique<Moon>(moonRadius, orbitRadius, orbitSpeed, moonColor, 16);
        planet.moons.push_back(std::move(moon));
    }
    
    spdlog::info("Generated {} moons for planet at ({:.1f}, {:.1f}, {:.1f})", 
                 moonCount, planet.position.x, planet.position.y, planet.position.z);
}
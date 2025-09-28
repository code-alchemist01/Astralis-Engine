#include "ConfigManager.hpp"
#include "Camera.hpp"
#include "SolarSystemManager.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <spdlog/spdlog.h>

bool ConfigManager::saveConfig(const std::string& filename, const Camera* camera, 
                              const SolarSystemManager* solarSystem) {
    if (!camera || !solarSystem) {
        spdlog::error("ConfigManager::saveConfig - Invalid camera or solar system pointer");
        return false;
    }
    
    AppConfig config;
    config.camera = cameraToConfig(camera);
    config.solarSystem = solarSystemToConfig(solarSystem);
    config.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::string jsonString = configToJson(config);
    bool success = writeJsonToFile(filename, jsonString);
    
    if (success) {
        lastConfig_ = config;
        spdlog::info("Configuration saved to: {}", filename);
    } else {
        spdlog::error("Failed to save configuration to: {}", filename);
    }
    
    return success;
}

bool ConfigManager::loadConfig(const std::string& filename, Camera* camera, 
                              SolarSystemManager* solarSystem) {
    if (!camera || !solarSystem) {
        spdlog::error("ConfigManager::loadConfig - Invalid camera or solar system pointer");
        return false;
    }
    
    std::string jsonString;
    if (!readJsonFromFile(filename, jsonString)) {
        spdlog::error("Failed to read configuration file: {}", filename);
        return false;
    }
    
    AppConfig config;
    if (!jsonToConfig(jsonString, config)) {
        spdlog::error("Failed to parse configuration file: {}", filename);
        return false;
    }
    
    configToCamera(config.camera, camera);
    configToSolarSystem(config.solarSystem, solarSystem);
    
    lastConfig_ = config;
    spdlog::info("Configuration loaded from: {}", filename);
    return true;
}

bool ConfigManager::saveCameraConfig(const std::string& filename, const Camera* camera) {
    if (!camera) {
        spdlog::error("ConfigManager::saveCameraConfig - Invalid camera pointer");
        return false;
    }
    
    CameraConfig config = cameraToConfig(camera);
    std::string jsonString = cameraConfigToJson(config);
    bool success = writeJsonToFile(filename, jsonString);
    
    if (success) {
        spdlog::info("Camera configuration saved to: {}", filename);
    } else {
        spdlog::error("Failed to save camera configuration to: {}", filename);
    }
    
    return success;
}

bool ConfigManager::loadCameraConfig(const std::string& filename, Camera* camera) {
    if (!camera) {
        spdlog::error("ConfigManager::loadCameraConfig - Invalid camera pointer");
        return false;
    }
    
    std::string jsonString;
    if (!readJsonFromFile(filename, jsonString)) {
        spdlog::error("Failed to read camera configuration file: {}", filename);
        return false;
    }
    
    AppConfig config;
    if (!jsonToConfig(jsonString, config)) {
        spdlog::error("Failed to parse camera configuration file: {}", filename);
        return false;
    }
    
    configToCamera(config.camera, camera);
    spdlog::info("Camera configuration loaded from: {}", filename);
    return true;
}

bool ConfigManager::isValidConfigFile(const std::string& filename) const {
    std::string jsonString;
    if (!readJsonFromFile(filename, jsonString)) {
        return false;
    }
    
    AppConfig config;
    return jsonToConfig(jsonString, config);
}

std::string ConfigManager::getDefaultSaveDirectory() const {
    return "saves";
}

bool ConfigManager::createDefaultSaveDirectory() const {
    try {
        std::filesystem::create_directories(getDefaultSaveDirectory());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create save directory: {}", e.what());
        return false;
    }
}

ConfigManager::CameraConfig ConfigManager::cameraToConfig(const Camera* camera) const {
    CameraConfig config;
    
    config.position = camera->getPosition();
    config.yaw = camera->getYaw();
    config.pitch = camera->getPitch();
    config.zoom = camera->getZoom();
    config.mode = static_cast<int>(camera->getMode());
    config.movementSpeed = camera->getMovementSpeed();
    config.mouseSensitivity = camera->getMouseSensitivity();
    config.motionBlurEnabled = camera->isMotionBlurEnabled();
    
    config.orbitDistance = camera->getOrbitDistance();
    config.orbitSpeed = camera->getOrbitSpeed();
    config.orbitHeight = camera->getOrbitHeight();
    
    config.autoFollowEnabled = camera->isAutoFollowEnabled();
    config.followDistance = camera->getFollowDistance();
    config.followHeight = camera->getFollowHeight();
    config.followSmoothing = camera->getFollowSmoothing();
    
    return config;
}

void ConfigManager::configToCamera(const CameraConfig& config, Camera* camera) const {
    camera->setPosition(config.position);
    camera->setYaw(config.yaw);
    camera->setPitch(config.pitch);
    camera->setZoom(config.zoom);
    camera->setMode(static_cast<Camera::Mode>(config.mode));
    camera->setMovementSpeed(config.movementSpeed);
    camera->setMouseSensitivity(config.mouseSensitivity);
    camera->enableMotionBlur(config.motionBlurEnabled);
    
    camera->setOrbitDistance(config.orbitDistance);
    camera->setOrbitSpeed(config.orbitSpeed);
    camera->setOrbitHeight(config.orbitHeight);
    
    camera->enableAutoFollow(config.autoFollowEnabled);
    camera->setFollowDistance(config.followDistance);
    camera->setFollowHeight(config.followHeight);
    camera->setFollowSmoothing(config.followSmoothing);
}

ConfigManager::SolarSystemConfig ConfigManager::solarSystemToConfig(const SolarSystemManager* solarSystem) const {
    SolarSystemConfig config;
    
    // Solar system settings
    config.seed = solarSystem->getSeed(); // Get actual current seed
    config.systemScale = solarSystem->getSystemScale();
    config.timeScale = solarSystem->getTimeScale();
    config.asteroidsVisible = solarSystem->getAsteroidBeltsVisible();
    config.ringsVisible = solarSystem->getPlanetaryRingsVisible();
    config.particlesVisible = solarSystem->getParticleSystemsVisible();
    config.asteroidDensity = solarSystem->getAsteroidDensity();
    config.ringDensity = solarSystem->getRingDensity();
    config.particleEmissionRate = solarSystem->getParticleEmissionRate();
    
    return config;
}

void ConfigManager::configToSolarSystem(const SolarSystemConfig& config, SolarSystemManager* solarSystem) const {
    // Regenerate solar system with the loaded seed
    solarSystem->generateSolarSystem(config.seed, 8); // Default planet count
    solarSystem->setSystemScale(config.systemScale);
    solarSystem->setTimeScale(config.timeScale);
    solarSystem->setAsteroidBeltsVisible(config.asteroidsVisible);
    solarSystem->setPlanetaryRingsVisible(config.ringsVisible);
    solarSystem->setParticleSystemsVisible(config.particlesVisible);
    solarSystem->setAsteroidDensity(config.asteroidDensity);
    solarSystem->setRingDensity(config.ringDensity);
    solarSystem->setParticleEmissionRate(config.particleEmissionRate);
}

bool ConfigManager::writeJsonToFile(const std::string& filename, const std::string& jsonString) const {
    try {
        // Create directory if it doesn't exist
        std::filesystem::path filePath(filename);
        std::filesystem::path dirPath = filePath.parent_path();
        
        if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
            if (!std::filesystem::create_directories(dirPath)) {
                spdlog::error("Failed to create directory: {}", dirPath.string());
                return false;
            }
            spdlog::info("Created directory: {}", dirPath.string());
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open file for writing: {}", filename);
            return false;
        }
        
        file << jsonString;
        file.close();
        
        // Verify file was written successfully
        if (std::filesystem::exists(filename)) {
            spdlog::info("Successfully wrote configuration to: {}", filename);
            return true;
        } else {
            spdlog::error("File was not created: {}", filename);
            return false;
        }
    } catch (const std::exception& e) {
        spdlog::error("Error writing to file {}: {}", filename, e.what());
        return false;
    }
}

bool ConfigManager::readJsonFromFile(const std::string& filename, std::string& jsonString) const {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        jsonString = buffer.str();
        file.close();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error reading from file {}: {}", filename, e.what());
        return false;
    }
}

std::string ConfigManager::configToJson(const AppConfig& config) const {
    std::stringstream json;
    
    json << "{\n";
    json << "  \"version\": \"" << config.version << "\",\n";
    json << "  \"timestamp\": " << config.timestamp << ",\n";
    
    // Camera configuration
    json << "  \"camera\": {\n";
    json << "    \"position\": [" << config.camera.position.x << ", " 
         << config.camera.position.y << ", " << config.camera.position.z << "],\n";
    json << "    \"yaw\": " << config.camera.yaw << ",\n";
    json << "    \"pitch\": " << config.camera.pitch << ",\n";
    json << "    \"zoom\": " << config.camera.zoom << ",\n";
    json << "    \"mode\": " << config.camera.mode << ",\n";
    json << "    \"movementSpeed\": " << config.camera.movementSpeed << ",\n";
    json << "    \"mouseSensitivity\": " << config.camera.mouseSensitivity << ",\n";
    json << "    \"motionBlurEnabled\": " << (config.camera.motionBlurEnabled ? "true" : "false") << ",\n";
    json << "    \"orbitDistance\": " << config.camera.orbitDistance << ",\n";
    json << "    \"orbitSpeed\": " << config.camera.orbitSpeed << ",\n";
    json << "    \"orbitHeight\": " << config.camera.orbitHeight << ",\n";
    json << "    \"autoFollowEnabled\": " << (config.camera.autoFollowEnabled ? "true" : "false") << ",\n";
    json << "    \"followDistance\": " << config.camera.followDistance << ",\n";
    json << "    \"followHeight\": " << config.camera.followHeight << ",\n";
    json << "    \"followSmoothing\": " << config.camera.followSmoothing << "\n";
    json << "  },\n";
    
    // Solar system configuration
    json << "  \"solarSystem\": {\n";
    json << "    \"seed\": " << config.solarSystem.seed << ",\n";
    json << "    \"systemScale\": " << config.solarSystem.systemScale << ",\n";
    json << "    \"timeScale\": " << config.solarSystem.timeScale << ",\n";
    json << "    \"asteroidsVisible\": " << (config.solarSystem.asteroidsVisible ? "true" : "false") << ",\n";
    json << "    \"ringsVisible\": " << (config.solarSystem.ringsVisible ? "true" : "false") << ",\n";
    json << "    \"particlesVisible\": " << (config.solarSystem.particlesVisible ? "true" : "false") << ",\n";
    json << "    \"asteroidDensity\": " << config.solarSystem.asteroidDensity << ",\n";
    json << "    \"ringDensity\": " << config.solarSystem.ringDensity << ",\n";
    json << "    \"particleEmissionRate\": " << config.solarSystem.particleEmissionRate << "\n";
    json << "  }\n";
    json << "}";
    
    return json.str();
}

std::string ConfigManager::cameraConfigToJson(const CameraConfig& config) const {
    std::stringstream json;
    
    json << "{\n";
    json << "  \"version\": \"1.0.0\",\n";
    json << "  \"timestamp\": " << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << ",\n";
    
    // Camera configuration only
    json << "  \"camera\": {\n";
    json << "    \"position\": [" << config.position.x << ", " 
         << config.position.y << ", " << config.position.z << "],\n";
    json << "    \"yaw\": " << config.yaw << ",\n";
    json << "    \"pitch\": " << config.pitch << ",\n";
    json << "    \"zoom\": " << config.zoom << ",\n";
    json << "    \"mode\": " << config.mode << ",\n";
    json << "    \"movementSpeed\": " << config.movementSpeed << ",\n";
    json << "    \"mouseSensitivity\": " << config.mouseSensitivity << ",\n";
    json << "    \"motionBlurEnabled\": " << (config.motionBlurEnabled ? "true" : "false") << ",\n";
    json << "    \"orbitDistance\": " << config.orbitDistance << ",\n";
    json << "    \"orbitSpeed\": " << config.orbitSpeed << ",\n";
    json << "    \"orbitHeight\": " << config.orbitHeight << ",\n";
    json << "    \"autoFollowEnabled\": " << (config.autoFollowEnabled ? "true" : "false") << ",\n";
    json << "    \"followDistance\": " << config.followDistance << ",\n";
    json << "    \"followHeight\": " << config.followHeight << ",\n";
    json << "    \"followSmoothing\": " << config.followSmoothing << "\n";
    json << "  }\n";
    json << "}";
    
    return json.str();
}

bool ConfigManager::jsonToConfig(const std::string& jsonString, AppConfig& config) const {
    try {
        // Simple JSON parsing without external library
        // This is a basic implementation - in production, use a proper JSON library
        
        // Find camera section
        size_t cameraStart = jsonString.find("\"camera\":");
        if (cameraStart == std::string::npos) return false;
        
        // Parse camera position
        size_t posStart = jsonString.find("\"position\":", cameraStart);
        if (posStart != std::string::npos) {
            size_t arrayStart = jsonString.find("[", posStart);
            size_t arrayEnd = jsonString.find("]", arrayStart);
            if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                std::string posStr = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                std::stringstream ss(posStr);
                std::string item;
                int i = 0;
                while (std::getline(ss, item, ',') && i < 3) {
                    float value = std::stof(item);
                    if (i == 0) config.camera.position.x = value;
                    else if (i == 1) config.camera.position.y = value;
                    else if (i == 2) config.camera.position.z = value;
                    i++;
                }
            }
        }
        
        // Parse other camera values
        auto parseFloat = [&](const std::string& key, float& value) {
            size_t keyPos = jsonString.find("\"" + key + "\":", cameraStart);
            if (keyPos != std::string::npos) {
                size_t valueStart = jsonString.find(":", keyPos) + 1;
                size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                    std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                    // Remove whitespace
                    valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                    valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                    value = std::stof(valueStr);
                }
            }
        };
        
        auto parseInt = [&](const std::string& key, int& value) {
            size_t keyPos = jsonString.find("\"" + key + "\":", cameraStart);
            if (keyPos != std::string::npos) {
                size_t valueStart = jsonString.find(":", keyPos) + 1;
                size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                    std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                    valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                    valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                    value = std::stoi(valueStr);
                }
            }
        };
        
        auto parseBool = [&](const std::string& key, bool& value) {
            size_t keyPos = jsonString.find("\"" + key + "\":", cameraStart);
            if (keyPos != std::string::npos) {
                size_t valueStart = jsonString.find(":", keyPos) + 1;
                size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                    std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                    valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                    valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                    value = (valueStr == "true");
                }
            }
        };
        
        parseFloat("yaw", config.camera.yaw);
        parseFloat("pitch", config.camera.pitch);
        parseFloat("zoom", config.camera.zoom);
        parseInt("mode", config.camera.mode);
        parseFloat("movementSpeed", config.camera.movementSpeed);
        parseFloat("mouseSensitivity", config.camera.mouseSensitivity);
        parseBool("motionBlurEnabled", config.camera.motionBlurEnabled);
        parseFloat("orbitDistance", config.camera.orbitDistance);
        parseFloat("orbitSpeed", config.camera.orbitSpeed);
        parseFloat("orbitHeight", config.camera.orbitHeight);
        parseBool("autoFollowEnabled", config.camera.autoFollowEnabled);
        parseFloat("followDistance", config.camera.followDistance);
        parseFloat("followHeight", config.camera.followHeight);
        parseFloat("followSmoothing", config.camera.followSmoothing);
        
        // Parse solar system section
        size_t solarSystemStart = jsonString.find("\"solarSystem\":");
        if (solarSystemStart != std::string::npos) {
            auto parseFloatSS = [&](const std::string& key, float& value) {
                size_t keyPos = jsonString.find("\"" + key + "\":", solarSystemStart);
                if (keyPos != std::string::npos) {
                    size_t valueStart = jsonString.find(":", keyPos) + 1;
                    size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                    if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                        std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                        value = std::stof(valueStr);
                    }
                }
            };
            
            auto parseIntSS = [&](const std::string& key, int& value) {
                size_t keyPos = jsonString.find("\"" + key + "\":", solarSystemStart);
                if (keyPos != std::string::npos) {
                    size_t valueStart = jsonString.find(":", keyPos) + 1;
                    size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                    if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                        std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                        value = std::stoi(valueStr);
                    }
                }
            };
            
            auto parseBoolSS = [&](const std::string& key, bool& value) {
                size_t keyPos = jsonString.find("\"" + key + "\":", solarSystemStart);
                if (keyPos != std::string::npos) {
                    size_t valueStart = jsonString.find(":", keyPos) + 1;
                    size_t valueEnd = jsonString.find_first_of(",\n}", valueStart);
                    if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                        std::string valueStr = jsonString.substr(valueStart, valueEnd - valueStart);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                        value = (valueStr == "true");
                    }
                }
            };
            
            parseIntSS("seed", config.solarSystem.seed);
            parseFloatSS("systemScale", config.solarSystem.systemScale);
            parseFloatSS("timeScale", config.solarSystem.timeScale);
            parseBoolSS("asteroidsVisible", config.solarSystem.asteroidsVisible);
            parseBoolSS("ringsVisible", config.solarSystem.ringsVisible);
            parseBoolSS("particlesVisible", config.solarSystem.particlesVisible);
            parseFloatSS("asteroidDensity", config.solarSystem.asteroidDensity);
            parseFloatSS("ringDensity", config.solarSystem.ringDensity);
            parseFloatSS("particleEmissionRate", config.solarSystem.particleEmissionRate);
        }
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error parsing JSON: {}", e.what());
        return false;
    }
}
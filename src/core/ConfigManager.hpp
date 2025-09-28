#pragma once

#include <string>
#include <glm/glm.hpp>
#include <memory>

// Forward declarations
class Camera;
class SolarSystemManager;

/**
 * @brief Configuration manager for saving and loading application state
 * 
 * This class handles saving and loading of camera settings, solar system
 * configurations, and other application state to/from JSON files.
 */
class ConfigManager {
public:
    /**
     * @brief Camera configuration structure
     */
    struct CameraConfig {
        glm::vec3 position{0.0f, 0.0f, 3.0f};
        float yaw{-90.0f};
        float pitch{0.0f};
        float zoom{45.0f};
        int mode{0}; // Camera::Mode as int
        float movementSpeed{2.5f};
        float mouseSensitivity{0.1f};
        bool motionBlurEnabled{false};
        
        // Orbit settings
        float orbitDistance{200.0f};
        float orbitSpeed{1.0f};
        float orbitHeight{0.0f};
        
        // Follow settings
        bool autoFollowEnabled{false};
        float followDistance{100.0f};
        float followHeight{20.0f};
        float followSmoothing{2.0f};
    };
    
    /**
     * @brief Solar system configuration structure
     */
    struct SolarSystemConfig {
        int seed{12345};
        float systemScale{1.0f};
        float timeScale{1.0f};
        bool asteroidsVisible{true};
        bool ringsVisible{true};
        bool particlesVisible{true};
        float asteroidDensity{1.0f};
        float ringDensity{1.0f};
        float particleEmissionRate{1.0f};
    };
    
    /**
     * @brief Complete application configuration
     */
    struct AppConfig {
        CameraConfig camera;
        SolarSystemConfig solarSystem;
        std::string version{"1.0.0"};
        long long timestamp{0};
    };
    
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    /**
     * @brief Save current application state to file
     * @param filename Path to save file
     * @param camera Camera instance to save
     * @param solarSystem Solar system manager to save
     * @return true if successful, false otherwise
     */
    bool saveConfig(const std::string& filename, const Camera* camera, 
                   const SolarSystemManager* solarSystem);
    
    /**
     * @brief Load application state from file
     * @param filename Path to load file
     * @param camera Camera instance to load into
     * @param solarSystem Solar system manager to load into
     * @return true if successful, false otherwise
     */
    bool loadConfig(const std::string& filename, Camera* camera, 
                   SolarSystemManager* solarSystem);
    
    /**
     * @brief Save only camera configuration
     * @param filename Path to save file
     * @param camera Camera instance to save
     * @return true if successful, false otherwise
     */
    bool saveCameraConfig(const std::string& filename, const Camera* camera);
    
    /**
     * @brief Load only camera configuration
     * @param filename Path to load file
     * @param camera Camera instance to load into
     * @return true if successful, false otherwise
     */
    bool loadCameraConfig(const std::string& filename, Camera* camera);
    
    /**
     * @brief Get the last loaded/saved configuration
     * @return const reference to app config
     */
    const AppConfig& getLastConfig() const { return lastConfig_; }
    
    /**
     * @brief Check if a config file exists and is valid
     * @param filename Path to config file
     * @return true if file exists and is valid JSON
     */
    bool isValidConfigFile(const std::string& filename) const;
    
    /**
     * @brief Get default save directory
     * @return string path to default save directory
     */
    std::string getDefaultSaveDirectory() const;
    
    /**
     * @brief Create default save directory if it doesn't exist
     * @return true if directory exists or was created successfully
     */
    bool createDefaultSaveDirectory() const;

private:
    AppConfig lastConfig_;
    
    /**
     * @brief Convert camera to config structure
     * @param camera Camera instance
     * @return CameraConfig structure
     */
    CameraConfig cameraToConfig(const Camera* camera) const;
    
    /**
     * @brief Apply config to camera
     * @param config Camera configuration
     * @param camera Camera instance to modify
     */
    void configToCamera(const CameraConfig& config, Camera* camera) const;
    
    /**
     * @brief Convert solar system to config structure
     * @param solarSystem Solar system manager
     * @return SolarSystemConfig structure
     */
    SolarSystemConfig solarSystemToConfig(const SolarSystemManager* solarSystem) const;
    
    /**
     * @brief Apply config to solar system
     * @param config Solar system configuration
     * @param solarSystem Solar system manager to modify
     */
    void configToSolarSystem(const SolarSystemConfig& config, SolarSystemManager* solarSystem) const;
    
    /**
     * @brief Write JSON string to file
     * @param filename Target file path
     * @param jsonString JSON content
     * @return true if successful
     */
    bool writeJsonToFile(const std::string& filename, const std::string& jsonString) const;
    
    /**
     * @brief Read JSON string from file
     * @param filename Source file path
     * @param jsonString Output JSON content
     * @return true if successful
     */
    bool readJsonFromFile(const std::string& filename, std::string& jsonString) const;
    
    /**
     * @brief Convert config to JSON string
     * @param config Application configuration
     * @return JSON string representation
     */
    std::string configToJson(const AppConfig& config) const;
    
    /**
     * @brief Convert camera config to JSON string (camera-only)
     * @param config Camera configuration
     * @return JSON string representation
     */
    std::string cameraConfigToJson(const CameraConfig& config) const;
    
    /**
     * @brief Parse JSON string to AppConfig
     * @param jsonString JSON content
     * @param config Output configuration
     * @return true if successful
     */
    bool jsonToConfig(const std::string& jsonString, AppConfig& config) const;
};
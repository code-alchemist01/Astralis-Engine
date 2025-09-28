#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Forward declarations
class Window;
class Shader;
class Camera;
class Geometry;
class Noise;
class Planet;
class PlanetManager;
class SolarSystemManager;
class ConfigManager;
namespace Core { 
    class InputManager; 
    class Texture;
}

class App {
public:
    App();
    ~App();

    // Non-copyable, non-movable
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    int run(int argc, char** argv);

private:
    void init();
    void loop();
    void shutdown();
    
    void processCommandLine(int argc, char** argv);
    void update(float deltaTime);
    void render();
    
    // ImGui methods
    void initImGui();
    void shutdownImGui();
    void renderImGui();
    
    // Utility methods
    std::string getCurrentTimeString() const;

    std::unique_ptr<Window> window_;
    std::unique_ptr<Shader> basicShader_;
    std::unique_ptr<Shader> texturedShader_;
    std::unique_ptr<Shader> skyboxShader_;
    std::unique_ptr<Shader> planetShader_;
    std::unique_ptr<Shader> sunShader_;
    std::unique_ptr<Shader> asteroidShader_;
    std::unique_ptr<Shader> ringShader_;
    std::unique_ptr<Shader> particleShader_;
    std::unique_ptr<Camera> camera_;

    std::unique_ptr<Geometry> skyboxGeometry_;
    std::unique_ptr<Core::Texture> checkerboardTexture_;
    std::unique_ptr<Core::Texture> brickTexture_;
    std::unique_ptr<Core::Texture> skyboxTexture_;
    std::unique_ptr<Noise> noise_;
    std::unique_ptr<SolarSystemManager> solarSystemManager_;
    std::unique_ptr<ConfigManager> configManager_;
    bool running_ = true;
    uint64_t seed_ = 1337; // Default seed
    
    // Starfield parameters
    bool useStarfield_ = true;
    float starDensity_ = 0.001f;
    float starBrightness_ = 1.0f;
    
    // Solar system parameters
    int planetCount_ = 8;
    int systemSeed_ = 1337;
    float maxRenderDistance_ = 500.0f;
};
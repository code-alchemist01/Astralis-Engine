#include "App.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Texture.hpp"
#include "Noise.hpp"
#include "Planet.hpp"
#include "PlanetManager.hpp"
#include "Sun.hpp"
#include "SolarSystemManager.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

App::App() = default;
App::~App() = default;

int App::run(int argc, char** argv) {
    spdlog::info("Procedural Universe Generator starting...");
    
    processCommandLine(argc, argv);
    init();
    loop();
    shutdown();
    
    spdlog::info("Application terminated successfully.");
    return 0;
}

void App::init() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initializing application...");
    
    // Create window
    window_ = std::make_unique<Window>(1280, 720, "Procedural Universe Generator");
    
    if (!window_->isValid()) {
        throw std::runtime_error("Failed to create window!");
    }
    
    spdlog::info("Window validation passed, testing OpenGL core profile...");
    
    // Make sure the OpenGL context is current
    glfwMakeContextCurrent(window_->getGLFWwindow());
    
    // Test basic OpenGL functionality without extension loader
    spdlog::info("Testing basic OpenGL calls...");
    
    // Try to get OpenGL version using core functions
    try {
        // These are core OpenGL functions that should work without extension loaders
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        
        if (version) {
            spdlog::info("OpenGL Version: {}", version);
        }
        if (renderer) {
            spdlog::info("OpenGL Renderer: {}", renderer);
        }
        if (vendor) {
            spdlog::info("OpenGL Vendor: {}", vendor);
        }
        
        // Set up basic OpenGL state using core functions
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        
        spdlog::info("OpenGL core profile working successfully!");
    }
    catch (const std::exception& e) {
        spdlog::error("OpenGL core profile test failed: {}", e.what());
        throw;
    }
    
    spdlog::info("OpenGL setup complete, initializing Input Manager...");
    
    // Initialize Input Manager
    Core::InputManager::getInstance().initialize(window_->getGLFWwindow());
    
    spdlog::info("Input Manager initialization complete");
    
    // Set up input callbacks for demonstration
    Core::InputManager::getInstance().setKeyCallback([this](int key, Core::KeyState state) {
        if (key == GLFW_KEY_ESCAPE && state == Core::KeyState::Pressed) {
            running_ = false;
            spdlog::info("ESC key pressed - exiting application");
        }
        if (key == GLFW_KEY_SPACE && state == Core::KeyState::Pressed) {
            spdlog::info("Space key pressed");
        }
    });
    
    Core::InputManager::getInstance().setMouseButtonCallback([](Core::MouseButton button, Core::KeyState state) {
        if (state == Core::KeyState::Pressed) {
            switch (button) {
                case Core::MouseButton::Left:
                    spdlog::info("Left mouse button pressed");
                    break;
                case Core::MouseButton::Right:
                    spdlog::info("Right mouse button pressed");
                    break;
                case Core::MouseButton::Middle:
                    spdlog::info("Middle mouse button pressed");
                    break;
            }
        }
    });
    
    Core::InputManager::getInstance().setMouseMoveCallback([](const Core::MousePosition& pos, const Core::MouseDelta& delta) {
        // Only log if there's significant movement to avoid spam
        if (std::abs(delta.deltaX) > 5.0 || std::abs(delta.deltaY) > 5.0) {
            spdlog::debug("Mouse moved to ({:.1f}, {:.1f}), delta: ({:.1f}, {:.1f})", 
                         pos.x, pos.y, delta.deltaX, delta.deltaY);
        }
    });
    
    // Initialize shader system
    spdlog::info("Initializing shader system...");
    try {
        basicShader_ = std::make_unique<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");
        spdlog::info("Basic shader created, checking validity...");
        if (!basicShader_->isValid()) {
            throw std::runtime_error("Failed to create basic shader");
        }
        
        spdlog::info("Basic shader validation passed, proceeding to textured shader...");
        spdlog::info("Loading textured shader...");
        try {
            texturedShader_ = std::make_unique<Shader>("assets/shaders/textured.vert", "assets/shaders/textured.frag");
            if (!texturedShader_->isValid()) {
                spdlog::error("Textured shader is not valid!");
                throw std::runtime_error("Failed to create textured shader");
            }
            spdlog::info("Textured shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during textured shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading skybox shader...");
        try {
            skyboxShader_ = std::make_unique<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
            if (!skyboxShader_->isValid()) {
                spdlog::error("Skybox shader is not valid!");
                throw std::runtime_error("Failed to create skybox shader");
            }
            spdlog::info("Skybox shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during skybox shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading planet shader...");
        try {
            planetShader_ = std::make_unique<Shader>("assets/shaders/planet.vert", "assets/shaders/planet.frag");
            if (!planetShader_->isValid()) {
                spdlog::error("Planet shader is not valid!");
                throw std::runtime_error("Failed to create planet shader");
            }
            spdlog::info("Planet shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during planet shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading sun shader...");
        try {
            sunShader_ = std::make_unique<Shader>("assets/shaders/sun.vert", "assets/shaders/sun.frag");
            if (!sunShader_->isValid()) {
                spdlog::error("Sun shader is not valid!");
                throw std::runtime_error("Failed to create sun shader");
            }
            spdlog::info("Sun shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during sun shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading asteroid shader...");
        try {
            asteroidShader_ = std::make_unique<Shader>("assets/shaders/asteroid.vert", "assets/shaders/asteroid.frag");
            if (!asteroidShader_->isValid()) {
                spdlog::error("Asteroid shader is not valid!");
                throw std::runtime_error("Failed to create asteroid shader");
            }
            spdlog::info("Asteroid shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during asteroid shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading ring shader...");
        try {
            ringShader_ = std::make_unique<Shader>("assets/shaders/ring.vert", "assets/shaders/ring.frag");
            if (!ringShader_->isValid()) {
                spdlog::error("Ring shader is not valid!");
                throw std::runtime_error("Failed to create ring shader");
            }
            spdlog::info("Ring shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during ring shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Loading particle shader...");
        try {
            particleShader_ = std::make_unique<Shader>("assets/shaders/particle.vert", "assets/shaders/particle.frag");
            if (!particleShader_->isValid()) {
                spdlog::error("Particle shader is not valid!");
                throw std::runtime_error("Failed to create particle shader");
            }
            spdlog::info("Particle shader loaded successfully");
        } catch (const std::exception& e) {
            spdlog::error("Exception during particle shader creation: {}", e.what());
            throw;
        }
        
        spdlog::info("Shader system initialized successfully");
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize shader system: {}", e.what());
        throw;
    }
    
    // Initialize camera system
    spdlog::info("Initializing camera system...");
    try {
        camera_ = std::make_unique<Camera>(
            glm::vec3(0.0f, 20.0f, 50.0f),  // position - moved back and up to see solar system
            glm::vec3(0.0f, 1.0f, 0.0f),    // up vector
            -90.0f,                          // yaw
            -15.0f                           // pitch - slight downward angle to see planets
        );
        spdlog::info("Camera system initialized successfully");
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize camera system: {}", e.what());
        throw;
    }
    
    // Initialize geometry system
    spdlog::info("Initializing geometry system...");
    try {

        
        // Create skybox geometry
        skyboxGeometry_ = std::make_unique<Geometry>();
        skyboxGeometry_->setVertices(Geometry::createSkyboxCube());
        skyboxGeometry_->setIndices(Geometry::createSkyboxIndices());
        skyboxGeometry_->uploadToGPU();
        
        spdlog::info("Geometry system initialized successfully");
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize geometry system: {}", e.what());
        throw;
    }
    
    // Initialize texture system
    spdlog::info("Initializing texture system...");
    try {
        checkerboardTexture_ = std::make_unique<Core::Texture>("assets/textures/checkerboard.png");
        brickTexture_ = std::make_unique<Core::Texture>("assets/textures/brick.png");
        
        // Load skybox cubemap texture - temporarily disabled for procedural starfield
        skyboxTexture_ = std::make_unique<Core::Texture>();
        // Create a dummy 1x1 white texture for now
        skyboxTexture_->createDummyTexture();
        
        spdlog::info("Texture system initialized successfully");
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize texture system: {}", e.what());
        throw;
    }
    
    // Initialize noise system
    spdlog::info("Initializing noise system...");
    try {
        noise_ = std::make_unique<Noise>(static_cast<int>(seed_));
        noise_->setNoiseType(Noise::NoiseType::OpenSimplex2);
        noise_->setFrequency(0.01f);
        noise_->setFractalType(Noise::FractalType::FBm);
        noise_->setFractalOctaves(4);
        noise_->setFractalLacunarity(2.0f);
        noise_->setFractalGain(0.5f);
        
        spdlog::info("Noise system initialized successfully with seed: {}", seed_);
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize noise system: {}", e.what());
        throw;
    }
    
    // Initialize solar system manager
    spdlog::info("Initializing solar system manager...");
    try {
        solarSystemManager_ = std::make_unique<SolarSystemManager>();
        solarSystemManager_->initialize(noise_.get());
        solarSystemManager_->generateSolarSystem(systemSeed_, planetCount_);
        
        spdlog::info("Solar system initialized successfully with {} planets", planetCount_);
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize solar system manager: {}", e.what());
        throw;
    }
    
    // Initialize ImGui
    initImGui();
    
    spdlog::info("Application initialized successfully.");
}

void App::loop() {
    spdlog::info("Entering main loop...");
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running_ && !window_->shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Poll events
        window_->pollEvents();
        
        // Update input manager
        Core::InputManager::getInstance().update();
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Swap buffers
        window_->swapBuffers();
        
        // Cap framerate to ~120 FPS
        std::this_thread::sleep_for(std::chrono::microseconds(8333));
    }
}

void App::shutdown() {
    spdlog::info("Shutting down application...");
    shutdownImGui();
    Core::InputManager::getInstance().shutdown();
    window_.reset();
}

void App::processCommandLine(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--seed" && i + 1 < argc) {
            try {
                seed_ = std::stoull(argv[i + 1]);
                spdlog::info("Using seed: {}", seed_);
                ++i; // Skip next argument
            }
            catch (const std::exception& e) {
                spdlog::warn("Invalid seed value: {}", argv[i + 1]);
            }
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Procedural Universe Generator\n";
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --seed <number>  Set generation seed (default: 1337)\n";
            std::cout << "  --help, -h       Show this help message\n";
            running_ = false;
            return;
        }
    }
}

void App::update(float deltaTime) {
    // Update camera with input
    if (camera_) {
        // Update camera systems (transitions, modes, effects)
        camera_->update(deltaTime);
        
        // Process keyboard input for camera movement
        auto& inputManager = Core::InputManager::getInstance();
        
        if (inputManager.isKeyHeld(GLFW_KEY_W)) {
            spdlog::debug("W key held - moving forward");
            camera_->processKeyboard(Camera::Movement::FORWARD, deltaTime);
        }
        if (inputManager.isKeyHeld(GLFW_KEY_S)) {
            spdlog::debug("S key held - moving backward");
            camera_->processKeyboard(Camera::Movement::BACKWARD, deltaTime);
        }
        if (inputManager.isKeyHeld(GLFW_KEY_A)) {
            spdlog::debug("A key held - moving left");
            camera_->processKeyboard(Camera::Movement::LEFT, deltaTime);
        }
        if (inputManager.isKeyHeld(GLFW_KEY_D)) {
            spdlog::debug("D key held - moving right");
            camera_->processKeyboard(Camera::Movement::RIGHT, deltaTime);
        }
        if (inputManager.isKeyHeld(GLFW_KEY_Q)) {
            camera_->processKeyboard(Camera::Movement::UP, deltaTime);
        }
        if (inputManager.isKeyHeld(GLFW_KEY_E)) {
            camera_->processKeyboard(Camera::Movement::DOWN, deltaTime);
        }
        
        // Speed boost with Shift
        if (inputManager.isKeyHeld(GLFW_KEY_LEFT_SHIFT)) {
            camera_->enableSpeedBoost(true);
        } else {
            camera_->enableSpeedBoost(false);
        }
        
        // Process mouse input for camera rotation (only when right mouse button is held)
        if (inputManager.isMouseButtonHeld(Core::MouseButton::Right)) {
            auto mousePos = inputManager.getMousePosition();
            static bool firstMouse = true;
            static float lastX = 640.0f;
            static float lastY = 360.0f;
            
            if (firstMouse) {
                lastX = mousePos.x;
                lastY = mousePos.y;
                firstMouse = false;
            }
            
            float xOffset = mousePos.x - lastX;
            float yOffset = lastY - mousePos.y; // Reversed since y-coordinates go from bottom to top
            lastX = mousePos.x;
            lastY = mousePos.y;
            
            camera_->processMouseMovement(xOffset, yOffset);
        }
    }
    
    // Update solar system
    if (solarSystemManager_) {
        solarSystemManager_->update(deltaTime);
    }
}

void App::render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render skybox first (before other objects)
    if (skyboxShader_ && skyboxShader_->isValid() && skyboxGeometry_ && skyboxGeometry_->isValid() && camera_) {
        // Disable face culling for skybox (we're inside the cube)
        glDisable(GL_CULL_FACE);
        
        // Change depth function to less equal for skybox
        glDepthFunc(GL_LEQUAL);
        
        skyboxShader_->use();
        
        // Get camera matrices (remove translation from view matrix)
        glm::mat4 view = glm::mat4(glm::mat3(camera_->getViewMatrix())); // Remove translation
        glm::mat4 projection = camera_->getProjectionMatrix(
            static_cast<float>(window_->getWidth()) / static_cast<float>(window_->getHeight())
        );
        
        // Send matrices to skybox shader
        skyboxShader_->setMat4("uView", view);
        skyboxShader_->setMat4("uProjection", projection);
        
        // Set starfield parameters
        spdlog::debug("Setting starfield uniforms: useStarfield={}, density={}, brightness={}, seed={}", 
                     useStarfield_, starDensity_, starBrightness_, seed_);
        skyboxShader_->setBool("uUseStarfield", useStarfield_);
        skyboxShader_->setFloat("uStarDensity", starDensity_);
        skyboxShader_->setFloat("uStarBrightness", starBrightness_);
        skyboxShader_->setUint("uSeed", static_cast<unsigned int>(seed_));
        
        // Bind skybox cubemap texture
        if (skyboxTexture_) {
            spdlog::debug("Binding skybox texture to unit 0");
            skyboxTexture_->bind(0);
            spdlog::debug("Setting uSkybox uniform to 0");
            skyboxShader_->setInt("uSkybox", 0);
        } else {
            spdlog::error("Skybox texture is null!");
        }
        
        // Render skybox
        skyboxGeometry_->draw();
        
        // Unbind texture
        if (skyboxTexture_) {
            skyboxTexture_->unbind();
        }
        
        skyboxShader_->unuse();
        
        // Re-enable face culling and reset depth function
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }
    

    

    
    // Render solar system (sun and planets)
    if (solarSystemManager_ && planetShader_ && sunShader_) {
        // Get camera matrices
        glm::mat4 view = camera_->getViewMatrix();
        glm::mat4 projection = camera_->getProjectionMatrix(
            static_cast<float>(window_->getWidth()) / static_cast<float>(window_->getHeight())
        );
        
        glm::vec3 viewPos = camera_->getPosition();
        
        // Render entire solar system (sun provides lighting for planets)
        solarSystemManager_->render(planetShader_.get(), sunShader_.get(), asteroidShader_.get(), 
                                   ringShader_.get(), particleShader_.get(), camera_.get(), view, projection, viewPos);
    }
    
    // Render ImGui
    renderImGui();
}

void App::initImGui() {
    spdlog::info("Initializing ImGui...");
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_->getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    spdlog::info("ImGui initialized successfully");
}

void App::shutdownImGui() {
    spdlog::info("Shutting down ImGui...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void App::renderImGui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Set up ImGui style for better appearance
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.Alpha = 0.95f;

    // Main control panel - compact and organized
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Astralis Engine Control Panel", nullptr, ImGuiWindowFlags_NoCollapse)) {
        
        // Tab bar for organized sections
        if (ImGui::BeginTabBar("ControlTabs")) {
            
            // Solar System Tab
            if (ImGui::BeginTabItem("Solar System")) {
                ImGui::Spacing();
                
                // Quick generation controls
                ImGui::Text("🌌 System Generation");
                ImGui::Separator();
                
                if (ImGui::SliderInt("Planets", &planetCount_, 3, 15)) {
                    if (solarSystemManager_) {
                        solarSystemManager_->generateSolarSystem(systemSeed_, planetCount_);
                    }
                }
                
                ImGui::PushItemWidth(200);
                if (ImGui::InputInt("Seed", &systemSeed_)) {
                    if (solarSystemManager_) {
                        solarSystemManager_->generateSolarSystem(systemSeed_, planetCount_);
                    }
                }
                ImGui::PopItemWidth();
                
                if (ImGui::Button("🎲 Random System", ImVec2(-1, 0))) {
                    systemSeed_ = static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
                    if (solarSystemManager_) {
                        solarSystemManager_->generateSolarSystem(systemSeed_, planetCount_);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Text("✨ Visual Effects");
                ImGui::Separator();
                
                if (solarSystemManager_) {
                    static bool asteroidsVisible = true;
                    ImGui::Checkbox("Asteroid Belts", &asteroidsVisible);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        solarSystemManager_->setAsteroidBeltsVisible(asteroidsVisible);
                    }
                    
                    static bool ringsVisible = true;
                    ImGui::Checkbox("Planetary Rings", &ringsVisible);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        solarSystemManager_->setPlanetaryRingsVisible(ringsVisible);
                    }
                    
                    static bool particlesVisible = true;
                    ImGui::Checkbox("Particle Effects", &particlesVisible);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        solarSystemManager_->setParticleSystemsVisible(particlesVisible);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Text("📊 System Info");
                ImGui::Separator();
                
                if (solarSystemManager_ && solarSystemManager_->getPlanetManager()) {
                    ImGui::Text("Planets: %zu", solarSystemManager_->getPlanetManager()->getPlanetCount());
                    ImGui::Text("Seed: %d", systemSeed_);
                    ImGui::Text("Render Distance: %.0f", maxRenderDistance_);
                }
                
                ImGui::EndTabItem();
            }
            
            // Camera Tab
            if (ImGui::BeginTabItem("Camera")) {
                ImGui::Spacing();
                
                // Camera mode selection
                ImGui::Text("📷 Camera Mode");
                ImGui::Separator();
                
                static int currentMode = 0;
                const char* modes[] = {"Free Fly", "Orbit", "Follow", "Cinematic"};
                
                if (ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
                    camera_->setMode(static_cast<Camera::Mode>(currentMode));
                }
                
                ImGui::Spacing();
                ImGui::Text("⚙️ Settings");
                ImGui::Separator();
                
                static float moveSpeed = 5.0f;
                static float mouseSensitivity = 0.1f;
                
                if (ImGui::SliderFloat("Speed", &moveSpeed, 0.1f, 50.0f)) {
                    camera_->setMovementSpeed(moveSpeed);
                }
                
                if (ImGui::SliderFloat("Sensitivity", &mouseSensitivity, 0.01f, 1.0f)) {
                    camera_->setMouseSensitivity(mouseSensitivity);
                }
                
                // Mode-specific controls
                if (currentMode == 1) { // Orbit mode
                    ImGui::Spacing();
                    ImGui::Text("🔄 Orbit Controls");
                    ImGui::Separator();
                    
                    static float orbitDistance = 200.0f;
                    static float orbitSpeed = 1.0f;
                    
                    if (ImGui::SliderFloat("Distance", &orbitDistance, 50.0f, 1000.0f)) {
                        camera_->setOrbitDistance(orbitDistance);
                    }
                    
                    if (ImGui::SliderFloat("Speed", &orbitSpeed, 0.1f, 5.0f)) {
                        camera_->setOrbitSpeed(orbitSpeed);
                    }
                    
                    if (ImGui::Button("Target Sun", ImVec2(-1, 0))) {
                        camera_->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                    }
                }
                
                ImGui::Spacing();
                ImGui::Text("🎬 Quick Actions");
                ImGui::Separator();
                
                if (ImGui::Button("System Tour", ImVec2(-1, 0))) {
                    std::vector<glm::vec3> waypoints = {
                        glm::vec3(0.0f, 0.0f, 500.0f),
                        glm::vec3(200.0f, 100.0f, 200.0f),
                        glm::vec3(0.0f, 200.0f, 0.0f),
                        glm::vec3(-200.0f, 50.0f, 200.0f),
                        glm::vec3(0.0f, 0.0f, 100.0f)
                    };
                    camera_->startCinematicPath(waypoints, 15.0f);
                    camera_->playCinematicSequence();
                }
                
                if (ImGui::Button("Reset Camera", ImVec2(-1, 0))) {
                    camera_->resetToDefault();
                }
                
                ImGui::EndTabItem();
            }
            
            // Rendering Tab
            if (ImGui::BeginTabItem("Rendering")) {
                ImGui::Spacing();
                
                ImGui::Text("⭐ Starfield");
                ImGui::Separator();
                
                ImGui::Checkbox("Enable Starfield", &useStarfield_);
                
                if (useStarfield_) {
                    if (ImGui::SliderFloat("Density", &starDensity_, 0.0001f, 0.01f, "%.4f")) {
                        // Starfield density updated
                    }
                    
                    if (ImGui::SliderFloat("Brightness", &starBrightness_, 0.1f, 3.0f)) {
                        // Starfield brightness updated
                    }
                }
                
                ImGui::Spacing();
                ImGui::Text("🔧 Render Settings");
                ImGui::Separator();
                
                if (ImGui::SliderFloat("Max Distance", &maxRenderDistance_, 100.0f, 2000.0f)) {
                    if (solarSystemManager_ && solarSystemManager_->getPlanetManager()) {
                        solarSystemManager_->getPlanetManager()->setMaxRenderDistance(maxRenderDistance_);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Text("📈 Performance");
                ImGui::Separator();
                
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
                
                ImGui::EndTabItem();
            }
            
            // Planet Details Tab
            if (ImGui::BeginTabItem("Planets")) {
                ImGui::Spacing();
                
                if (solarSystemManager_ && solarSystemManager_->getPlanetManager() && 
                    solarSystemManager_->getPlanetManager()->getPlanetCount() > 0) {
                    
                    ImGui::Text("🪐 Planet Inspector");
                    ImGui::Separator();
                    
                    static int selectedPlanet = 0;
                    int maxPlanets = static_cast<int>(solarSystemManager_->getPlanetManager()->getPlanetCount()) - 1;
                    
                    if (ImGui::SliderInt("Select", &selectedPlanet, 0, maxPlanets)) {
                        // Planet selection changed
                    }
                    
                    PlanetInstance* planet = solarSystemManager_->getPlanetManager()->getPlanet(selectedPlanet);
                    if (planet) {
                        ImGui::Spacing();
                        
                        // Planet type display
                        const char* typeNames[] = {"🪨 Rocky", "🌪️ Gas Giant", "🧊 Ice", "🏜️ Desert"};
                        const char* typeName = (planet->type >= 0 && planet->type < 4) ? typeNames[planet->type] : "❓ Unknown";
                        ImGui::Text("Type: %s", typeName);
                        
                        ImGui::Text("Position: (%.0f, %.0f, %.0f)", 
                                   planet->position.x, planet->position.y, planet->position.z);
                        ImGui::Text("Scale: %.2f", planet->scale);
                        ImGui::Text("Seed: %d", planet->seed);
                        
                        ImGui::ColorEdit3("Color", &planet->color.x, ImGuiColorEditFlags_NoInputs);
                        
                        if (planet->planet && planet->planet->getGeometry()) {
                            ImGui::Separator();
                            ImGui::Text("Geometry Info:");
                            ImGui::Text("Vertices: %d", planet->planet->getGeometry()->getVertexCount());
                            ImGui::Text("Resolution: %d", planet->planet->getResolution());
                        }
                    }
                } else {
                    ImGui::Text("No planets available");
                    ImGui::Text("Generate a solar system first!");
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // Compact status overlay
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 120), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250, 110), ImGuiCond_Always);
    
    if (ImGui::Begin("Status", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {
        
        glm::vec3 pos = camera_->getPosition();
        ImGui::Text("📍 Position: (%.0f, %.0f, %.0f)", pos.x, pos.y, pos.z);
        
        ImGui::Text("🎯 Mode: %s", camera_->isTransitioning() ? "Transitioning" : "Active");
        
        if (camera_->isCinematicPlaying()) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "🎬 Cinematic Playing");
        }
        
        // Quick camera controls
        if (ImGui::Button("🏠", ImVec2(30, 25))) {
            camera_->resetToDefault();
        }
        ImGui::SameLine();
        if (ImGui::Button("☀️", ImVec2(30, 25))) {
            camera_->transitionToTarget(glm::vec3(0.0f, 0.0f, 0.0f), 300.0f, 2.0f, Camera::TransitionType::EASE_IN_OUT);
        }
        ImGui::SameLine();
        if (ImGui::Button("🔄", ImVec2(30, 25))) {
            camera_->setMode(Camera::Mode::ORBIT);
            camera_->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("🎬", ImVec2(30, 25))) {
            if (camera_->isCinematicPlaying()) {
                camera_->stopCinematicSequence();
            } else {
                std::vector<glm::vec3> waypoints = {
                    glm::vec3(0.0f, 0.0f, 500.0f),
                    glm::vec3(200.0f, 100.0f, 200.0f),
                    glm::vec3(0.0f, 200.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 100.0f)
                };
                camera_->startCinematicPath(waypoints, 12.0f);
                camera_->playCinematicSequence();
            }
        }
    }
    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
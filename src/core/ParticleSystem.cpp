#include "ParticleSystem.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <random>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// OpenGL function pointers (same as in Geometry.cpp)
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef void GLvoid;

// OpenGL constants
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#endif
#ifndef GL_FLOAT
#define GL_FLOAT 0x1406
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif
#ifndef GL_BLEND
#define GL_BLEND 0x0BE2
#endif
#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA 0x0302
#endif
#ifndef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#endif
#ifndef GL_ONE
#define GL_ONE 1
#endif

// OpenGL function pointers
static void (*glGenVertexArrays)(GLsizei n, GLuint *arrays) = nullptr;
static void (*glBindVertexArray)(GLuint array) = nullptr;
static void (*glDeleteVertexArrays)(GLsizei n, const GLuint *arrays) = nullptr;
static void (*glGenBuffers)(GLsizei n, GLuint *buffers) = nullptr;
static void (*glBindBuffer)(GLenum target, GLuint buffer) = nullptr;
static void (*glBufferData)(GLenum target, GLsizei size, const GLvoid *data, GLenum usage) = nullptr;
static void (*glBufferSubData)(GLenum target, GLint offset, GLsizei size, const GLvoid *data) = nullptr;
static void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers) = nullptr;
static void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, unsigned char normalized, GLsizei stride, const GLvoid *pointer) = nullptr;
static void (*glEnableVertexAttribArray)(GLuint index) = nullptr;
static void (*glEnable)(GLenum cap) = nullptr;
static void (*glDisable)(GLenum cap) = nullptr;
static void (*glBlendFunc)(GLenum sfactor, GLenum dfactor) = nullptr;
static void (*glDrawArrays)(GLenum mode, GLint first, GLsizei count) = nullptr;

static bool particleFunctionsLoaded = false;

static void loadParticleOpenGLFunctions() {
    if (particleFunctionsLoaded) return;
    
    glGenVertexArrays = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (void(*)(GLuint))glfwGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenBuffers");
    glBindBuffer = (void(*)(GLenum, GLuint))glfwGetProcAddress("glBindBuffer");
    glBufferData = (void(*)(GLenum, GLsizei, const GLvoid*, GLenum))glfwGetProcAddress("glBufferData");
    glBufferSubData = (void(*)(GLenum, GLint, GLsizei, const GLvoid*))glfwGetProcAddress("glBufferSubData");
    glDeleteBuffers = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (void(*)(GLuint, GLint, GLenum, unsigned char, GLsizei, const GLvoid*))glfwGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (void(*)(GLuint))glfwGetProcAddress("glEnableVertexAttribArray");
    glEnable = (void(*)(GLenum))glfwGetProcAddress("glEnable");
    glDisable = (void(*)(GLenum))glfwGetProcAddress("glDisable");
    glBlendFunc = (void(*)(GLenum, GLenum))glfwGetProcAddress("glBlendFunc");
    glDrawArrays = (void(*)(GLenum, GLint, GLsizei))glfwGetProcAddress("glDrawArrays");
    
    if (glGenVertexArrays && glBindVertexArray && glDeleteVertexArrays && 
        glGenBuffers && glBindBuffer && glBufferData && glBufferSubData &&
        glDeleteBuffers && glVertexAttribPointer && glEnableVertexAttribArray &&
        glEnable && glDisable && glBlendFunc && glDrawArrays) {
        particleFunctionsLoaded = true;
        spdlog::info("Particle OpenGL functions loaded successfully");
    } else {
        spdlog::error("Failed to load Particle OpenGL functions");
    }
}

ParticleSystem::ParticleSystem(const glm::vec3& origin, ParticleType type, int maxParticles)
    : origin_(origin)
    , type_(type)
    , maxParticles_(maxParticles)
    , activeParticles_(0)
    , active_(true)
    , emissionRate_(50.0f)
    , emissionTimer_(0.0f)
    , gravityStrength_(0.1f)
    , magneticFieldStrength_(0.05f)
    , solarWindStrength_(0.02f)
    , temperatureDecay_(0.95f)
    , maxRenderDistance_(1000.0f)
    , useTemperatureColoring_(true)
    , useBloom_(true)
    , VAO_(0)
    , VBO_(0)
    , instanceVBO_(0)
    , buffersInitialized_(false) {
    
    loadParticleOpenGLFunctions();
    particles_.reserve(maxParticles_);
    
    // Set type-specific parameters
    switch (type_) {
        case ParticleType::SOLAR_FLARE:
            emissionRate_ = 100.0f;
            gravityStrength_ = 0.05f;
            magneticFieldStrength_ = 0.1f;
            break;
        case ParticleType::COSMIC_DUST:
            emissionRate_ = 20.0f;
            gravityStrength_ = 0.02f;
            magneticFieldStrength_ = 0.01f;
            break;
        case ParticleType::STELLAR_WIND:
            emissionRate_ = 200.0f;
            gravityStrength_ = 0.001f;
            magneticFieldStrength_ = 0.03f;
            break;
        case ParticleType::CORONA_PARTICLES:
            emissionRate_ = 150.0f;
            gravityStrength_ = 0.08f;
            magneticFieldStrength_ = 0.15f;
            break;
    }
}

ParticleSystem::~ParticleSystem() {
    cleanupBuffers();
}

void ParticleSystem::initialize() {
    setupRenderingBuffers();
    spdlog::info("ParticleSystem initialized with {} max particles", maxParticles_);
}

void ParticleSystem::update(float deltaTime) {
    if (!active_) return;
    
    // Update emission timer
    emissionTimer_ += deltaTime;
    
    // Emit new particles based on emission rate
    if (emissionTimer_ >= 1.0f / emissionRate_ && activeParticles_ < maxParticles_) {
        emissionTimer_ = 0.0f;
        
        // Emit particles based on type
        switch (type_) {
            case ParticleType::SOLAR_FLARE:
                emitSolarFlare(origin_, glm::normalize(glm::vec3(1.0f, 0.5f, 0.0f)), 1.0f, 0.8f);
                break;
            case ParticleType::COSMIC_DUST:
                emitCosmicDust(origin_, 50.0f, 5);
                break;
            case ParticleType::STELLAR_WIND:
                emitStellarWind(origin_, 10.0f, 0.5f);
                break;
            case ParticleType::CORONA_PARTICLES:
                emitParticles(10, origin_, glm::vec3(0.0f, 1.0f, 0.0f), 0.3f);
                break;
        }
    }
    
    // Update all particles
    for (auto& particle : particles_) {
        if (particle.life > 0.0f) {
            updateParticlePhysics(particle, deltaTime);
        }
    }
    
    // Remove dead particles
    removeDeadParticles();
}

void ParticleSystem::updateParticlePhysics(Particle& particle, float deltaTime) {
    // Update life
    particle.life -= deltaTime;
    if (particle.life <= 0.0f) return;
    
    // Update based on particle type
    switch (particle.type) {
        case ParticleType::SOLAR_FLARE:
            updateSolarFlareParticle(particle, deltaTime);
            break;
        case ParticleType::COSMIC_DUST:
            updateCosmicDustParticle(particle, deltaTime);
            break;
        case ParticleType::STELLAR_WIND:
            updateStellarWindParticle(particle, deltaTime);
            break;
        case ParticleType::CORONA_PARTICLES:
            updateCoronaParticle(particle, deltaTime);
            break;
    }
    
    // Apply common physics
    particle.velocity += particle.acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;
    
    // Update alpha based on life
    float lifeRatio = particle.life / particle.maxLife;
    particle.alpha = lifeRatio * 0.8f;
    
    // Update temperature decay
    particle.temperature *= temperatureDecay_;
    
    // Update color based on temperature if enabled
    if (useTemperatureColoring_) {
        particle.color = calculateTemperatureColor(particle.temperature);
    }
}

void ParticleSystem::updateSolarFlareParticle(Particle& particle, float deltaTime) {
    // Apply magnetic field effects
    applyMagneticForce(particle, deltaTime);
    
    // Solar flares follow magnetic field lines
    glm::vec3 magneticDirection = glm::normalize(particle.position - origin_);
    particle.acceleration += magneticDirection * particle.magneticField * magneticFieldStrength_;
    
    // Intensity affects size and brightness
    particle.size *= (1.0f + particle.intensity * 0.1f);
    particle.alpha *= (1.0f + particle.intensity * 0.5f);
}

void ParticleSystem::updateCosmicDustParticle(Particle& particle, float deltaTime) {
    // Cosmic dust is affected by gravity and solar wind
    glm::vec3 toOrigin = origin_ - particle.position;
    float distance = glm::length(toOrigin);
    
    if (distance > 0.0f) {
        // Gravity
        glm::vec3 gravityForce = glm::normalize(toOrigin) * gravityStrength_ / (distance * distance);
        particle.acceleration += gravityForce;
        
        // Solar wind pushes dust away
        applySolarWindForce(particle, deltaTime);
    }
    
    // Dust particles slowly rotate
    particle.velocity += glm::vec3(
        sin(particle.life * 2.0f) * 0.01f,
        cos(particle.life * 1.5f) * 0.01f,
        sin(particle.life * 1.8f) * 0.01f
    );
}

void ParticleSystem::updateStellarWindParticle(Particle& particle, float deltaTime) {
    // Stellar wind particles move radially outward from the sun
    glm::vec3 fromOrigin = particle.position - origin_;
    float distance = glm::length(fromOrigin);
    
    if (distance > 0.0f) {
        glm::vec3 windDirection = glm::normalize(fromOrigin);
        particle.acceleration += windDirection * solarWindStrength_;
        
        // Wind speed increases with distance from sun
        float speedMultiplier = 1.0f + distance * 0.001f;
        particle.velocity *= speedMultiplier;
    }
}

void ParticleSystem::updateCoronaParticle(Particle& particle, float deltaTime) {
    // Corona particles orbit around the sun with some randomness
    glm::vec3 toOrigin = origin_ - particle.position;
    float distance = glm::length(toOrigin);
    
    if (distance > 0.0f) {
        // Orbital motion
        glm::vec3 tangent = glm::cross(glm::normalize(toOrigin), glm::vec3(0.0f, 1.0f, 0.0f));
        particle.acceleration += tangent * 0.05f;
        
        // Magnetic confinement
        applyMagneticForce(particle, deltaTime);
        
        // Random thermal motion
        particle.acceleration += glm::vec3(
            (rand() / float(RAND_MAX) - 0.5f) * 0.02f,
            (rand() / float(RAND_MAX) - 0.5f) * 0.02f,
            (rand() / float(RAND_MAX) - 0.5f) * 0.02f
        );
    }
}

void ParticleSystem::applyMagneticForce(Particle& particle, float deltaTime) {
    // Simplified magnetic field effect
    glm::vec3 toOrigin = origin_ - particle.position;
    float distance = glm::length(toOrigin);
    
    if (distance > 0.0f) {
        glm::vec3 magneticField = glm::normalize(toOrigin) * magneticFieldStrength_ / distance;
        glm::vec3 magneticForce = glm::cross(particle.velocity, magneticField) * particle.magneticField;
        particle.acceleration += magneticForce;
    }
}

void ParticleSystem::applySolarWindForce(Particle& particle, float deltaTime) {
    glm::vec3 fromOrigin = particle.position - origin_;
    float distance = glm::length(fromOrigin);
    
    if (distance > 0.0f) {
        glm::vec3 windDirection = glm::normalize(fromOrigin);
        float windForce = solarWindStrength_ / (1.0f + distance * 0.01f);
        particle.acceleration += windDirection * windForce;
    }
}

glm::vec3 ParticleSystem::calculateTemperatureColor(float temperature) {
    // Simplified blackbody radiation color calculation
    temperature = std::clamp(temperature, 1000.0f, 10000.0f);
    
    float normalizedTemp = (temperature - 1000.0f) / 9000.0f;
    
    glm::vec3 color;
    if (normalizedTemp < 0.5f) {
        // Red to yellow
        color.r = 1.0f;
        color.g = normalizedTemp * 2.0f;
        color.b = 0.0f;
    } else {
        // Yellow to white to blue
        float t = (normalizedTemp - 0.5f) * 2.0f;
        color.r = 1.0f;
        color.g = 1.0f;
        color.b = t;
    }
    
    return color;
}

void ParticleSystem::emitParticles(int count, const glm::vec3& emissionPoint, 
                                  const glm::vec3& direction, float spread) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> spreadDist(-spread, spread);
    std::uniform_real_distribution<float> speedDist(1.0f, 5.0f);
    std::uniform_real_distribution<float> sizeDist(0.1f, 0.5f);
    std::uniform_real_distribution<float> lifeDist(2.0f, 8.0f);
    
    for (int i = 0; i < count && activeParticles_ < maxParticles_; ++i) {
        glm::vec3 velocity = direction + glm::vec3(
            spreadDist(rng),
            spreadDist(rng),
            spreadDist(rng)
        );
        velocity = glm::normalize(velocity) * speedDist(rng);
        
        glm::vec3 color = glm::vec3(1.0f, 0.8f, 0.4f); // Default warm color
        float size = sizeDist(rng);
        float life = lifeDist(rng);
        
        spawnParticle(emissionPoint, velocity, color, size, life);
    }
}

void ParticleSystem::emitSolarFlare(const glm::vec3& sunPosition, const glm::vec3& direction, 
                                   float intensity, float magneticStrength) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> spreadDist(-0.2f, 0.2f);
    std::uniform_real_distribution<float> speedDist(5.0f, 15.0f);
    std::uniform_real_distribution<float> sizeDist(0.2f, 1.0f);
    std::uniform_real_distribution<float> lifeDist(3.0f, 10.0f);
    
    int particleCount = static_cast<int>(intensity * 20.0f);
    
    for (int i = 0; i < particleCount && activeParticles_ < maxParticles_; ++i) {
        glm::vec3 velocity = direction + glm::vec3(
            spreadDist(rng),
            spreadDist(rng),
            spreadDist(rng)
        );
        velocity = glm::normalize(velocity) * speedDist(rng);
        
        Particle particle;
        particle.position = sunPosition;
        particle.velocity = velocity;
        particle.acceleration = glm::vec3(0.0f);
        particle.color = glm::vec3(1.0f, 0.6f, 0.2f); // Orange-red flare color
        particle.size = sizeDist(rng);
        particle.life = lifeDist(rng);
        particle.maxLife = particle.life;
        particle.alpha = 0.8f;
        particle.temperature = 5000.0f + intensity * 2000.0f;
        particle.type = ParticleType::SOLAR_FLARE;
        particle.intensity = intensity;
        particle.magneticField = magneticStrength;
        
        particles_.push_back(particle);
        activeParticles_++;
    }
}

void ParticleSystem::emitCosmicDust(const glm::vec3& center, float radius, int count) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, radius);
    std::uniform_real_distribution<float> speedDist(0.1f, 1.0f);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.2f);
    std::uniform_real_distribution<float> lifeDist(10.0f, 30.0f);
    
    for (int i = 0; i < count && activeParticles_ < maxParticles_; ++i) {
        float angle = angleDist(rng);
        float r = radiusDist(rng);
        
        glm::vec3 position = center + glm::vec3(
            r * cos(angle),
            (rng() / float(RAND_MAX) - 0.5f) * radius * 0.1f,
            r * sin(angle)
        );
        
        glm::vec3 velocity = glm::vec3(
            (rng() / float(RAND_MAX) - 0.5f) * speedDist(rng),
            (rng() / float(RAND_MAX) - 0.5f) * speedDist(rng),
            (rng() / float(RAND_MAX) - 0.5f) * speedDist(rng)
        );
        
        Particle particle;
        particle.position = position;
        particle.velocity = velocity;
        particle.acceleration = glm::vec3(0.0f);
        particle.color = glm::vec3(0.6f, 0.5f, 0.4f); // Dusty brown color
        particle.size = sizeDist(rng);
        particle.life = lifeDist(rng);
        particle.maxLife = particle.life;
        particle.alpha = 0.3f;
        particle.temperature = 300.0f;
        particle.type = ParticleType::COSMIC_DUST;
        particle.density = 0.5f;
        particle.reflectivity = 0.3f;
        
        particles_.push_back(particle);
        activeParticles_++;
    }
}

void ParticleSystem::emitStellarWind(const glm::vec3& sunPosition, float windSpeed, float density) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> speedDist(windSpeed * 0.8f, windSpeed * 1.2f);
    std::uniform_real_distribution<float> sizeDist(0.02f, 0.1f);
    std::uniform_real_distribution<float> lifeDist(5.0f, 15.0f);
    
    int particleCount = static_cast<int>(density * 30.0f);
    
    for (int i = 0; i < particleCount && activeParticles_ < maxParticles_; ++i) {
        float theta = angleDist(rng);
        float phi = angleDist(rng);
        
        glm::vec3 direction = glm::vec3(
            sin(phi) * cos(theta),
            cos(phi),
            sin(phi) * sin(theta)
        );
        
        glm::vec3 velocity = direction * speedDist(rng);
        
        Particle particle;
        particle.position = sunPosition + direction * 2.0f; // Start slightly away from sun
        particle.velocity = velocity;
        particle.acceleration = glm::vec3(0.0f);
        particle.color = glm::vec3(0.8f, 0.9f, 1.0f); // Bluish wind color
        particle.size = sizeDist(rng);
        particle.life = lifeDist(rng);
        particle.maxLife = particle.life;
        particle.alpha = 0.2f;
        particle.temperature = 1000000.0f; // Very hot plasma
        particle.type = ParticleType::STELLAR_WIND;
        
        particles_.push_back(particle);
        activeParticles_++;
    }
}

void ParticleSystem::spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                  const glm::vec3& color, float size, float life) {
    if (activeParticles_ >= maxParticles_) return;
    
    Particle particle;
    particle.position = position;
    particle.velocity = velocity;
    particle.acceleration = glm::vec3(0.0f);
    particle.color = color;
    particle.size = size;
    particle.life = life;
    particle.maxLife = life;
    particle.alpha = 0.8f;
    particle.temperature = 3000.0f;
    particle.type = type_;
    
    particles_.push_back(particle);
    activeParticles_++;
}

void ParticleSystem::removeDeadParticles() {
    auto it = std::remove_if(particles_.begin(), particles_.end(),
        [](const Particle& p) { return p.life <= 0.0f; });
    
    int removedCount = std::distance(it, particles_.end());
    particles_.erase(it, particles_.end());
    activeParticles_ -= removedCount;
}

void ParticleSystem::render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                           const glm::mat4& projection, const glm::vec3& lightPos, 
                           const glm::vec3& lightColor, const glm::vec3& viewPos) {
    if (!active_ || particles_.empty() || !buffersInitialized_) return;
    
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("viewPos", viewPos);
    
    // Enable blending for particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write to depth buffer
    
    glBindVertexArray(VAO_);
    
    int particlesRendered = 0;
    for (const auto& particle : particles_) {
        if (particle.life <= 0.0f) continue;
        
        // Distance culling
        float distance = glm::length(particle.position - viewPos);
        if (distance > maxRenderDistance_) continue;
        
        // Billboard transformation
        glm::vec3 right = glm::normalize(glm::cross(camera->getFront(), camera->getUp()));
        glm::vec3 up = glm::cross(right, camera->getFront());
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle.position);
        
        // Billboard rotation
        model[0] = glm::vec4(right * particle.size, 0.0f);
        model[1] = glm::vec4(up * particle.size, 0.0f);
        model[2] = glm::vec4(-camera->getFront() * particle.size, 0.0f);
        
        // Set uniforms
        shader->setMat4("model", model);
        shader->setVec3("particleColor", particle.color);
        shader->setFloat("alpha", particle.alpha);
        shader->setFloat("temperature", particle.temperature);
        shader->setFloat("intensity", particle.intensity);
        
        // Render particle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        particlesRendered++;
    }
    
    glBindVertexArray(0);
    shader->unuse();
    
    // Restore depth writing
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    // Log rendering stats occasionally
    static int frameCount = 0;
    if (++frameCount % 300 == 0) { // Every 5 seconds at 60 FPS
        spdlog::debug("Rendered {}/{} particles (type: {})", 
                     particlesRendered, particles_.size(), static_cast<int>(type_));
    }
}

void ParticleSystem::setupRenderingBuffers() {
    if (buffersInitialized_) {
        cleanupBuffers();
    }
    
    // Create a simple quad for each particle
    float quadVertices[] = {
        // positions        // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO_);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    buffersInitialized_ = true;
    spdlog::debug("ParticleSystem rendering buffers initialized");
}

void ParticleSystem::cleanupBuffers() {
    if (buffersInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        if (instanceVBO_ != 0) {
            glDeleteBuffers(1, &instanceVBO_);
        }
        buffersInitialized_ = false;
    }
}

void ParticleSystem::setPhysicsParameters(float gravity, float magneticField, float solarWind) {
    gravityStrength_ = gravity;
    magneticFieldStrength_ = magneticField;
    solarWindStrength_ = solarWind;
}
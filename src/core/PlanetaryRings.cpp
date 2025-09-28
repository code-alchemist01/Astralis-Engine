#include "PlanetaryRings.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <random>
#include <algorithm>
#include <spdlog/spdlog.h>

// OpenGL type definitions
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
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
#ifndef GL_TRUE
#define GL_TRUE 1
#endif
#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
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

// OpenGL function pointers
static void (*glGenVertexArrays)(GLsizei n, GLuint *arrays) = nullptr;
static void (*glBindVertexArray)(GLuint array) = nullptr;
static void (*glDeleteVertexArrays)(GLsizei n, const GLuint *arrays) = nullptr;
static void (*glGenBuffers)(GLsizei n, GLuint *buffers) = nullptr;
static void (*glBindBuffer)(GLenum target, GLuint buffer) = nullptr;
static void (*glBufferData)(GLenum target, GLsizei size, const GLvoid *data, GLenum usage) = nullptr;
static void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers) = nullptr;
static void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, unsigned char normalized, GLsizei stride, const GLvoid *pointer) = nullptr;
static void (*glEnableVertexAttribArray)(GLuint index) = nullptr;
static void (*glDrawElements_)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) = nullptr;
static void (*glEnable_)(GLenum cap) = nullptr;
static void (*glDisable_)(GLenum cap) = nullptr;
static void (*glBlendFunc_)(GLenum sfactor, GLenum dfactor) = nullptr;
static void (*glDepthMask_)(unsigned char flag) = nullptr;

static bool planetaryRingsFunctionsLoaded = false;

static void loadPlanetaryRingsOpenGLFunctions() {
    if (planetaryRingsFunctionsLoaded) return;

    glGenVertexArrays = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (void(*)(GLuint))glfwGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenBuffers");
    glBindBuffer = (void(*)(GLenum, GLuint))glfwGetProcAddress("glBindBuffer");
    glBufferData = (void(*)(GLenum, GLsizei, const GLvoid*, GLenum))glfwGetProcAddress("glBufferData");
    glDeleteBuffers = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (void(*)(GLuint, GLint, GLenum, unsigned char, GLsizei, const GLvoid*))glfwGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (void(*)(GLuint))glfwGetProcAddress("glEnableVertexAttribArray");
    glDrawElements_ = (void(*)(GLenum, GLsizei, GLenum, const GLvoid*))glfwGetProcAddress("glDrawElements");
    glEnable_ = (void(*)(GLenum))glfwGetProcAddress("glEnable");
    glDisable_ = (void(*)(GLenum))glfwGetProcAddress("glDisable");
    glBlendFunc_ = (void(*)(GLenum, GLenum))glfwGetProcAddress("glBlendFunc");
    glDepthMask_ = (void(*)(unsigned char))glfwGetProcAddress("glDepthMask");

    if (glGenVertexArrays && glBindVertexArray && glDeleteVertexArrays &&
        glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers &&
        glVertexAttribPointer && glEnableVertexAttribArray && glDrawElements_ &&
        glEnable_ && glDisable_ && glBlendFunc_ && glDepthMask_) {
        planetaryRingsFunctionsLoaded = true;
        spdlog::info("PlanetaryRings OpenGL functions loaded successfully");
    } else {
        spdlog::error("Failed to load PlanetaryRings OpenGL functions");
    }
}

PlanetaryRings::PlanetaryRings(const glm::vec3& planetPosition, float planetRadius,
                               float innerRadius, float outerRadius, int particleCount, int seed)
    : planetPosition_(planetPosition)
    , planetRadius_(planetRadius)
    , innerRadius_(innerRadius)
    , outerRadius_(outerRadius)
    , particleCount_(particleCount)
    , seed_(seed)
    , visible_(true)
    , orbitSpeedMultiplier_(1.0f)
    , opacityMultiplier_(1.0f)
    , maxRenderDistance_(2000.0f)
    , VAO_(0)
    , VBO_(0)
    , instanceVBO_(0)
    , buffersInitialized_(false)
{
    loadPlanetaryRingsOpenGLFunctions();
    generateRingParticles();
    spdlog::info("Created planetary rings: inner={:.1f}, outer={:.1f}, particles={}", 
                 innerRadius_, outerRadius_, particleCount_);
}

PlanetaryRings::~PlanetaryRings() {
    cleanupBuffers();
}

void PlanetaryRings::initialize() {
    setupRenderingBuffers();
}

void PlanetaryRings::generateRingParticles() {
    particles_.clear();
    particles_.reserve(particleCount_);

    std::mt19937 rng(seed_);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> radiusDist(innerRadius_, outerRadius_);
    std::uniform_real_distribution<float> heightDist(-0.2f, 0.2f); // Very thin rings
    std::uniform_real_distribution<float> sizeDist(0.02f, 0.1f);
    std::uniform_real_distribution<float> orbitSpeedDist(0.5f, 2.0f);
    std::uniform_real_distribution<float> colorVariation(0.6f, 1.0f);
    std::uniform_real_distribution<float> alphaDist(0.3f, 0.8f);

    for (int i = 0; i < particleCount_; ++i) {
        RingParticle particle;
        
        // Orbital parameters
        particle.orbitRadius = radiusDist(rng);
        particle.orbitAngle = angleDist(rng);
        
        // Kepler's laws: inner particles orbit faster
        float normalizedRadius = (particle.orbitRadius - innerRadius_) / (outerRadius_ - innerRadius_);
        particle.orbitSpeed = orbitSpeedDist(rng) / (1.0f + normalizedRadius * 2.0f);
        
        // Position (will be updated in updateParticlePositions)
        float height = heightDist(rng);
        particle.position = planetPosition_ + glm::vec3(
            particle.orbitRadius * cos(particle.orbitAngle),
            height,
            particle.orbitRadius * sin(particle.orbitAngle)
        );
        
        // Size
        particle.size = sizeDist(rng);
        
        // Color (ice/rock particles - bluish-white to brown)
        float iceRatio = 1.0f - normalizedRadius; // More ice in inner rings
        float colorVar = colorVariation(rng);
        if (iceRatio > 0.5f) {
            // Ice particles (bluish-white)
            particle.color = glm::vec3(
                colorVar * 0.9f,
                colorVar * 0.95f,
                colorVar
            );
        } else {
            // Rocky particles (brownish)
            particle.color = glm::vec3(
                colorVar * 0.8f,
                colorVar * 0.6f,
                colorVar * 0.4f
            );
        }
        
        // Alpha for transparency
        particle.alpha = alphaDist(rng);
        
        particles_.push_back(particle);
    }
}

void PlanetaryRings::update(float deltaTime) {
    if (!visible_) return;
    
    updateParticlePositions(deltaTime);
}

void PlanetaryRings::updateParticlePositions(float deltaTime) {
    for (auto& particle : particles_) {
        // Update orbital position
        particle.orbitAngle += particle.orbitSpeed * orbitSpeedMultiplier_ * deltaTime;
        
        // Keep angle in range [0, 2π]
        if (particle.orbitAngle > 2.0f * 3.14159f) {
            particle.orbitAngle -= 2.0f * 3.14159f;
        }
        
        // Update position based on orbital mechanics
        particle.position = planetPosition_ + glm::vec3(
            particle.orbitRadius * cos(particle.orbitAngle),
            particle.position.y - planetPosition_.y, // Keep relative height
            particle.orbitRadius * sin(particle.orbitAngle)
        );
    }
}

void PlanetaryRings::setupRenderingBuffers() {
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

    unsigned int EBO;
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceVBO_);

    glBindVertexArray(VAO_);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Instance data will be set up during rendering
    glBindVertexArray(0);

    buffersInitialized_ = true;
    spdlog::debug("Initialized planetary rings rendering buffers");
}

void PlanetaryRings::cleanupBuffers() {
    if (buffersInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &instanceVBO_);
        VAO_ = VBO_ = instanceVBO_ = 0;
        buffersInitialized_ = false;
    }
}

void PlanetaryRings::render(Shader* shader, const Camera* camera, const glm::mat4& view, 
                           const glm::mat4& projection, const glm::vec3& lightPos, 
                           const glm::vec3& lightColor, const glm::vec3& viewPos) {
    if (!visible_ || !shader || !camera || !buffersInitialized_ || particles_.empty()) {
        return;
    }

    glm::vec3 cameraPos = camera->getPosition();
    float distanceToPlanet = glm::length(planetPosition_ - cameraPos);
    
    // Skip if too far away
    if (distanceToPlanet > maxRenderDistance_) {
        return;
    }

    // Enable blending for transparency
    glEnable_(GL_BLEND);
    glBlendFunc_(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask_(GL_FALSE); // Don't write to depth buffer for transparent objects

    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("viewPos", viewPos);

    glBindVertexArray(VAO_);

    int particlesRendered = 0;
    for (const auto& particle : particles_) {
        float distance = glm::length(particle.position - cameraPos);
        
        // Skip particles that are too far away
        if (distance > maxRenderDistance_ * 0.5f) {
            continue;
        }

        // Create billboard matrix (always face camera)
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
        shader->setVec3("planetColor", particle.color);
        shader->setFloat("alpha", particle.alpha * opacityMultiplier_);

        // Render particle
        glDrawElements_(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        particlesRendered++;
    }

    glBindVertexArray(0);
    shader->unuse();

    // Restore depth writing
    glDepthMask_(GL_TRUE);
    glDisable_(GL_BLEND);

    // Log rendering stats occasionally
    static int frameCount = 0;
    if (++frameCount % 300 == 0) { // Every 5 seconds at 60 FPS
        spdlog::debug("Rendered {}/{} ring particles", particlesRendered, particles_.size());
    }
}

void PlanetaryRings::setDensity(float density) {
    // Clamp density between 0.1 and 3.0
    density = std::clamp(density, 0.1f, 3.0f);
    
    int newCount = static_cast<int>(particleCount_ * density);
    if (newCount != static_cast<int>(particles_.size())) {
        particleCount_ = newCount;
        generateRingParticles();
        spdlog::info("Updated planetary rings density: new count = {}", particleCount_);
    }
}
#include "Geometry.hpp"
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

// OpenGL function pointers (same as in Shader.cpp)
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
#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
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

static bool geometryFunctionsLoaded = false;

static void loadGeometryOpenGLFunctions() {
    if (geometryFunctionsLoaded) return;
    
    glGenVertexArrays = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (void(*)(GLuint))glfwGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenBuffers");
    glBindBuffer = (void(*)(GLenum, GLuint))glfwGetProcAddress("glBindBuffer");
    glBufferData = (void(*)(GLenum, GLsizei, const GLvoid*, GLenum))glfwGetProcAddress("glBufferData");
    glDeleteBuffers = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (void(*)(GLuint, GLint, GLenum, unsigned char, GLsizei, const GLvoid*))glfwGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (void(*)(GLuint))glfwGetProcAddress("glEnableVertexAttribArray");
    
    if (glGenVertexArrays && glBindVertexArray && glDeleteVertexArrays && 
        glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers &&
        glVertexAttribPointer && glEnableVertexAttribArray) {
        geometryFunctionsLoaded = true;
        spdlog::info("Geometry OpenGL functions loaded successfully");
    } else {
        spdlog::error("Failed to load Geometry OpenGL functions");
    }
}

Geometry::Geometry() {
    loadGeometryOpenGLFunctions();
}

Geometry::~Geometry() {
    cleanup();
}

void Geometry::cleanup() {
    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
        VAO_ = 0;
    }
    if (VBO_ != 0) {
        glDeleteBuffers(1, &VBO_);
        VBO_ = 0;
    }
    if (EBO_ != 0) {
        glDeleteBuffers(1, &EBO_);
        EBO_ = 0;
    }
}

void Geometry::setVertices(const std::vector<Vertex>& vertices) {
    vertices_ = vertices;
}

void Geometry::setIndices(const std::vector<unsigned int>& indices) {
    indices_ = indices;
    useIndices_ = !indices.empty();
}

void Geometry::uploadToGPU() {
    if (!geometryFunctionsLoaded) {
        spdlog::error("OpenGL functions not loaded for Geometry");
        return;
    }
    
    if (vertices_.empty()) {
        spdlog::error("No vertices to upload");
        return;
    }
    
    // Clean up existing buffers
    cleanup();
    
    // Generate VAO
    glGenVertexArrays(1, &VAO_);
    glBindVertexArray(VAO_);
    
    // Generate and bind VBO
    glGenBuffers(1, &VBO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);
    
    // Generate and bind EBO if using indices
    if (useIndices_) {
        glGenBuffers(1, &EBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
    }
    
    // Set vertex attribute pointers
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    
    // Texture coordinate attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    
    // Unbind VAO
    glBindVertexArray(0);
    
    spdlog::info("Geometry uploaded to GPU: {} vertices, {} indices", vertices_.size(), indices_.size());
}

void Geometry::bind() const {
    if (VAO_ != 0) {
        glBindVertexArray(VAO_);
    }
}

void Geometry::unbind() const {
    glBindVertexArray(0);
}

void Geometry::draw() const {
    if (VAO_ == 0) {
        spdlog::warn("Attempting to draw geometry that hasn't been uploaded to GPU");
        return;
    }
    
    bind();
    
    if (useIndices_ && !indices_.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
    }
    
    unbind();
}

// Static utility functions for creating basic shapes
std::vector<Geometry::Vertex> Geometry::createTriangle() {
    return {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( 0.0f,  0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f))
    };
}

std::vector<Geometry::Vertex> Geometry::createQuad() {
    return {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f))
    };
}

std::vector<Geometry::Vertex> Geometry::createCube() {
    return {
        // Front face
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
        
        // Back face
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
        
        // Left face
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        
        // Right face
        Vertex(glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        
        // Bottom face
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        
        // Top face
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f))
    };
}

std::vector<unsigned int> Geometry::createCubeIndices() {
    return {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Bottom face
        16, 17, 18, 18, 19, 16,
        // Top face
        20, 21, 22, 22, 23, 20
    };
}

std::vector<Geometry::Vertex> Geometry::createSkyboxCube() {
    // Skybox cube vertices (simplified, no normals/texcoords needed)
    // Winding order is reversed for inside-out rendering
    return {
        // Front face
        Vertex(glm::vec3(-1.0f, -1.0f,  1.0f)),
        Vertex(glm::vec3( 1.0f, -1.0f,  1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f,  1.0f)),
        Vertex(glm::vec3(-1.0f,  1.0f,  1.0f)),
        
        // Back face
        Vertex(glm::vec3( 1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3(-1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3(-1.0f,  1.0f, -1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f, -1.0f)),
        
        // Left face
        Vertex(glm::vec3(-1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3(-1.0f, -1.0f,  1.0f)),
        Vertex(glm::vec3(-1.0f,  1.0f,  1.0f)),
        Vertex(glm::vec3(-1.0f,  1.0f, -1.0f)),
        
        // Right face
        Vertex(glm::vec3( 1.0f, -1.0f,  1.0f)),
        Vertex(glm::vec3( 1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f, -1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f,  1.0f)),
        
        // Bottom face
        Vertex(glm::vec3(-1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3( 1.0f, -1.0f, -1.0f)),
        Vertex(glm::vec3( 1.0f, -1.0f,  1.0f)),
        Vertex(glm::vec3(-1.0f, -1.0f,  1.0f)),
        
        // Top face
        Vertex(glm::vec3(-1.0f,  1.0f,  1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f,  1.0f)),
        Vertex(glm::vec3( 1.0f,  1.0f, -1.0f)),
        Vertex(glm::vec3(-1.0f,  1.0f, -1.0f))
    };
}

std::vector<unsigned int> Geometry::createSkyboxIndices() {
    // Skybox indices with reversed winding order for inside-out rendering
    return {
        // Front face (reversed)
        0, 2, 1, 0, 3, 2,
        // Back face (reversed)
        4, 6, 5, 4, 7, 6,
        // Left face (reversed)
        8, 10, 9, 8, 11, 10,
        // Right face (reversed)
        12, 14, 13, 12, 15, 14,
        // Bottom face (reversed)
        16, 18, 17, 16, 19, 18,
        // Top face (reversed)
        20, 22, 21, 20, 23, 22
    };
}

void Geometry::createSphere(float radius, int latSegments, int lonSegments) {
    auto vertices = createSphereVertices(radius, latSegments, lonSegments);
    auto indices = createSphereIndices(latSegments, lonSegments);
    
    setVertices(vertices);
    setIndices(indices);
    uploadToGPU();
}

std::vector<Geometry::Vertex> Geometry::createSphereVertices(float radius, int latSegments, int lonSegments) {
    std::vector<Vertex> vertices;
    
    const float PI = 3.14159265359f;
    
    for (int lat = 0; lat <= latSegments; ++lat) {
        float theta = lat * PI / latSegments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        
        for (int lon = 0; lon <= lonSegments; ++lon) {
            float phi = lon * 2 * PI / lonSegments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            
            glm::vec3 position(
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi
            );
            
            glm::vec3 normal = glm::normalize(position);
            
            glm::vec2 texCoords(
                (float)lon / lonSegments,
                (float)lat / latSegments
            );
            
            vertices.emplace_back(position, normal, texCoords);
        }
    }
    
    return vertices;
}

std::vector<unsigned int> Geometry::createSphereIndices(int latSegments, int lonSegments) {
    std::vector<unsigned int> indices;
    
    for (int lat = 0; lat < latSegments; ++lat) {
        for (int lon = 0; lon < lonSegments; ++lon) {
            int first = lat * (lonSegments + 1) + lon;
            int second = first + lonSegments + 1;
            
            // First triangle
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            // Second triangle
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    return indices;
}
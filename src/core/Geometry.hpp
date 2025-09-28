#pragma once

#include <vector>
#include <glm/glm.hpp>

class Geometry {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        
        Vertex(const glm::vec3& pos, const glm::vec3& norm = glm::vec3(0.0f), const glm::vec2& tex = glm::vec2(0.0f))
            : position(pos), normal(norm), texCoords(tex) {}
    };

    Geometry();
    ~Geometry();

    // Non-copyable, non-movable for now
    Geometry(const Geometry&) = delete;
    Geometry& operator=(const Geometry&) = delete;
    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;

    // Setup geometry data
    void setVertices(const std::vector<Vertex>& vertices);
    void setIndices(const std::vector<unsigned int>& indices);
    void uploadToGPU();

    // Rendering
    void bind() const;
    void unbind() const;
    void draw() const;

    // Utility functions for creating basic shapes
    static std::vector<Vertex> createTriangle();
    static std::vector<Vertex> createQuad();
    static std::vector<Vertex> createCube();
    static std::vector<unsigned int> createCubeIndices();
    static std::vector<Vertex> createSkyboxCube();
    static std::vector<unsigned int> createSkyboxIndices();
    
    // Sphere creation
    void createSphere(float radius, int latSegments = 32, int lonSegments = 32);
    static std::vector<Vertex> createSphereVertices(float radius, int latSegments = 32, int lonSegments = 32);
    static std::vector<unsigned int> createSphereIndices(int latSegments = 32, int lonSegments = 32);

    // Getters
    bool isValid() const { return VAO_ != 0; }
    size_t getVertexCount() const { return vertices_.size(); }
    size_t getIndexCount() const { return indices_.size(); }

private:
    void cleanup();

    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    
    unsigned int VAO_ = 0;  // Vertex Array Object
    unsigned int VBO_ = 0;  // Vertex Buffer Object
    unsigned int EBO_ = 0;  // Element Buffer Object
    
    bool useIndices_ = false;
};
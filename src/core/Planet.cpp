#include "Planet.hpp"
#include "Geometry.hpp"
#include "Noise.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Planet::Planet(float radius, int resolution, Noise* noise)
    : radius_(radius)
    , resolution_(resolution)
    , noise_(noise)
    , geometry_(std::make_unique<Geometry>())
    , heightScale_(1.0f)
    , noiseFrequency_(0.01f)
    , noiseOctaves_(4)
    , orbitalRadius_(0.0f)
    , orbitalSpeed_(0.0f)
    , orbitalAngle_(0.0f)
    , orbitalPosition_(0.0f, 0.0f, 0.0f)
    , needsRegeneration_(true) {
}

void Planet::generate() {
    if (!needsRegeneration_) {
        return;
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<unsigned int> indices;

    // Reserve memory for efficiency
    const int verticesPerFace = resolution_ * resolution_;
    const int totalVertices = verticesPerFace * 6;
    const int indicesPerFace = (resolution_ - 1) * (resolution_ - 1) * 6;
    const int totalIndices = indicesPerFace * 6;

    vertices.reserve(totalVertices);
    normals.reserve(totalVertices);
    texCoords.reserve(totalVertices);
    indices.reserve(totalIndices);

    // Generate all 6 faces of the cube
    for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
        Face face = static_cast<Face>(faceIndex);
        unsigned int vertexOffset = faceIndex * verticesPerFace;
        generateFace(face, vertices, normals, texCoords, indices, vertexOffset);
    }

    // Convert to Vertex format for Geometry class
    std::vector<Geometry::Vertex> geometryVertices;
    geometryVertices.reserve(vertices.size());

    for (size_t i = 0; i < vertices.size(); ++i) {
        Geometry::Vertex vertex(vertices[i], normals[i], texCoords[i]);
        geometryVertices.push_back(vertex);
    }

    // Set geometry data
    geometry_->setVertices(geometryVertices);
    geometry_->setIndices(indices);
    geometry_->uploadToGPU();

    needsRegeneration_ = false;
}

void Planet::setRadius(float radius) {
    if (radius_ != radius) {
        radius_ = radius;
        needsRegeneration_ = true;
    }
}

void Planet::setResolution(int resolution) {
    if (resolution_ != resolution) {
        resolution_ = std::max(2, resolution); // Minimum resolution of 2
        needsRegeneration_ = true;
    }
}

void Planet::setNoise(Noise* noise) {
    noise_ = noise;
    needsRegeneration_ = true;
}

glm::vec3 Planet::cubeToSphere(Face face, float u, float v) const {
    // Convert u, v from [0, 1] to [-1, 1]
    float x = 2.0f * u - 1.0f;
    float y = 2.0f * v - 1.0f;

    glm::vec3 cubePos;

    // Map to cube face
    switch (face) {
        case Face::PositiveX: // Right face
            cubePos = glm::vec3(1.0f, -y, -x);
            break;
        case Face::NegativeX: // Left face
            cubePos = glm::vec3(-1.0f, -y, x);
            break;
        case Face::PositiveY: // Top face
            cubePos = glm::vec3(x, 1.0f, y);
            break;
        case Face::NegativeY: // Bottom face
            cubePos = glm::vec3(x, -1.0f, -y);
            break;
        case Face::PositiveZ: // Front face
            cubePos = glm::vec3(x, -y, 1.0f);
            break;
        case Face::NegativeZ: // Back face
            cubePos = glm::vec3(-x, -y, -1.0f);
            break;
    }

    // Project cube position to sphere
    // This is the key cube-to-sphere projection formula
    float x2 = cubePos.x * cubePos.x;
    float y2 = cubePos.y * cubePos.y;
    float z2 = cubePos.z * cubePos.z;

    glm::vec3 spherePos;
    spherePos.x = cubePos.x * std::sqrt(1.0f - y2 * 0.5f - z2 * 0.5f + y2 * z2 / 3.0f);
    spherePos.y = cubePos.y * std::sqrt(1.0f - z2 * 0.5f - x2 * 0.5f + z2 * x2 / 3.0f);
    spherePos.z = cubePos.z * std::sqrt(1.0f - x2 * 0.5f - y2 * 0.5f + x2 * y2 / 3.0f);

    return glm::normalize(spherePos);
}

float Planet::generateHeight(const glm::vec3& position) const {
    if (!noise_) {
        return 0.0f;
    }

    // Use multiple octaves of noise for realistic terrain
    float height = 0.0f;
    float amplitude = 1.0f;
    float frequency = noiseFrequency_;
    float maxValue = 0.0f;

    for (int i = 0; i < noiseOctaves_; ++i) {
        // Sample noise at the sphere position
        float noiseValue = noise_->get3D(
            position.x * frequency,
            position.y * frequency,
            position.z * frequency
        );

        height += noiseValue * amplitude;
        maxValue += amplitude;

        amplitude *= 0.5f;  // Reduce amplitude for each octave
        frequency *= 2.0f;  // Increase frequency for each octave
    }

    // Normalize and scale
    height /= maxValue;
    return height * heightScale_;
}

void Planet::generateFace(Face face, std::vector<glm::vec3>& vertices,
                         std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords,
                         std::vector<unsigned int>& indices, unsigned int vertexOffset) {
    
    // Generate vertices for this face
    for (int y = 0; y < resolution_; ++y) {
        for (int x = 0; x < resolution_; ++x) {
            // Calculate UV coordinates
            float u = static_cast<float>(x) / (resolution_ - 1);
            float v = static_cast<float>(y) / (resolution_ - 1);

            // Convert to sphere position
            glm::vec3 spherePos = cubeToSphere(face, u, v);

            // Generate height displacement
            float height = generateHeight(spherePos);

            // Apply height displacement
            glm::vec3 finalPos = spherePos * (radius_ + height);

            // Calculate normal (for now, use the sphere normal)
            glm::vec3 normal = calculateNormal(spherePos);

            // Add vertex data
            vertices.push_back(finalPos);
            normals.push_back(normal);
            texCoords.push_back(glm::vec2(u, v));
        }
    }

    // Generate indices for this face
    for (int y = 0; y < resolution_ - 1; ++y) {
        for (int x = 0; x < resolution_ - 1; ++x) {
            // Calculate vertex indices for the quad
            unsigned int topLeft = vertexOffset + y * resolution_ + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = topLeft + resolution_;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle (top-left, bottom-left, top-right)
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle (top-right, bottom-left, bottom-right)
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

glm::vec3 Planet::calculateNormal(const glm::vec3& position) const {
    if (!noise_) {
        return glm::normalize(position);
    }

    // Calculate normal using finite differences
    const float epsilon = 0.001f;
    
    glm::vec3 pos = glm::normalize(position);
    
    // Sample height at current position and nearby positions
    float heightCenter = generateHeight(pos);
    
    // Calculate tangent vectors by sampling nearby points
    glm::vec3 tangentU = glm::vec3(1, 0, 0);
    glm::vec3 tangentV = glm::vec3(0, 1, 0);
    
    // For simplicity, use the sphere normal for now
    // In a more advanced implementation, you would calculate the actual surface normal
    // by taking the cross product of tangent vectors derived from height samples
    
    return glm::normalize(pos);
}

void Planet::setOrbitalParameters(float radius, float speed) {
    orbitalRadius_ = radius;
    orbitalSpeed_ = speed;
    // Update initial position
    updateOrbit(0.0f);
}

void Planet::updateOrbit(float deltaTime) {
    // Update orbital angle
    orbitalAngle_ += orbitalSpeed_ * deltaTime;
    
    // Keep angle in [0, 2π] range
    while (orbitalAngle_ > 2.0f * M_PI) {
        orbitalAngle_ -= 2.0f * M_PI;
    }
    while (orbitalAngle_ < 0.0f) {
        orbitalAngle_ += 2.0f * M_PI;
    }
    
    // Calculate new position (circular orbit in XZ plane)
    orbitalPosition_.x = orbitalRadius_ * std::cos(orbitalAngle_);
    orbitalPosition_.y = 0.0f; // Keep planets in the same plane
    orbitalPosition_.z = orbitalRadius_ * std::sin(orbitalAngle_);
}
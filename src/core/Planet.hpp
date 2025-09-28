#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Forward declarations
class Geometry;
class Noise;

/**
 * @brief Planet class implementing cube-to-sphere projection for procedural planet generation
 * 
 * This class generates a planet using cube-to-sphere projection technique where each face
 * of a cube is subdivided and projected onto a sphere surface. Height displacement is
 * applied using noise functions to create realistic terrain.
 */
class Planet {
public:
    /**
     * @brief Planet face enumeration for cube-to-sphere projection
     */
    enum class Face {
        PositiveX = 0,  // Right face
        NegativeX = 1,  // Left face
        PositiveY = 2,  // Top face
        NegativeY = 3,  // Bottom face
        PositiveZ = 4,  // Front face
        NegativeZ = 5   // Back face
    };

    /**
     * @brief Construct a new Planet object
     * @param radius Planet radius in world units
     * @param resolution Resolution per face (vertices per edge)
     * @param noise Noise generator for height displacement
     */
    Planet(float radius = 100.0f, int resolution = 64, Noise* noise = nullptr);

    /**
     * @brief Destroy the Planet object
     */
    ~Planet() = default;

    // Non-copyable, non-movable for now
    Planet(const Planet&) = delete;
    Planet& operator=(const Planet&) = delete;
    Planet(Planet&&) = delete;
    Planet& operator=(Planet&&) = delete;

    /**
     * @brief Generate planet geometry using cube-to-sphere projection
     */
    void generate();

    /**
     * @brief Get the planet geometry for rendering
     * @return std::shared_ptr<Geometry> Planet geometry
     */
    Geometry* getGeometry() const { return geometry_.get(); }

    /**
     * @brief Set planet radius
     * @param radius New radius value
     */
    void setRadius(float radius);

    /**
     * @brief Set resolution per face
     * @param resolution New resolution value
     */
    void setResolution(int resolution);

    /**
     * @brief Set noise generator
     * @param noise Noise generator for height displacement
     */
    void setNoise(Noise* noise);

    /**
     * @brief Get planet radius
     * @return float Current radius
     */
    float getRadius() const { return radius_; }

    /**
     * @brief Get resolution per face
     * @return int Current resolution
     */
    int getResolution() const { return resolution_; }

    /**
     * @brief Set height scale for noise displacement
     * @param scale Height scale multiplier
     */
    void setHeightScale(float scale) { heightScale_ = scale; }

    /**
     * @brief Get height scale
     * @return float Current height scale
     */
    float getHeightScale() const { return heightScale_; }

    /**
     * @brief Set noise frequency for terrain generation
     * @param frequency Noise frequency
     */
    void setNoiseFrequency(float frequency) { noiseFrequency_ = frequency; }

    /**
     * @brief Get noise frequency
     * @return float Current noise frequency
     */
    float getNoiseFrequency() const { return noiseFrequency_; }

    /**
     * @brief Set noise octaves for terrain generation
     * @param octaves Number of noise octaves
     */
    void setNoiseOctaves(int octaves) { noiseOctaves_ = octaves; }

    /**
     * @brief Get noise octaves
     * @return int Current noise octaves
     */
    int getNoiseOctaves() const { return noiseOctaves_; }

private:
    /**
     * @brief Convert cube coordinates to sphere coordinates
     * @param face Cube face
     * @param u U coordinate on face [0, 1]
     * @param v V coordinate on face [0, 1]
     * @return glm::vec3 Normalized sphere position
     */
    glm::vec3 cubeToSphere(Face face, float u, float v) const;

    /**
     * @brief Generate height displacement using noise
     * @param position Normalized sphere position
     * @return float Height displacement value
     */
    float generateHeight(const glm::vec3& position) const;

    /**
     * @brief Generate vertices for a single face
     * @param face Face to generate
     * @param vertices Output vertex array
     * @param indices Output index array
     * @param vertexOffset Starting vertex index offset
     */
    void generateFace(Face face, std::vector<glm::vec3>& vertices, 
                     std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords,
                     std::vector<unsigned int>& indices, unsigned int vertexOffset);

    /**
     * @brief Calculate normal vector for a vertex
     * @param position Vertex position
     * @return glm::vec3 Normal vector
     */
    glm::vec3 calculateNormal(const glm::vec3& position) const;

private:
    float radius_;                          ///< Planet radius
    int resolution_;                        ///< Resolution per face
    Noise* noise_;                          ///< Noise generator
    std::unique_ptr<Geometry> geometry_;   ///< Planet geometry
    
    // Terrain generation parameters
    float heightScale_;                     ///< Height displacement scale
    float noiseFrequency_;                  ///< Base noise frequency
    int noiseOctaves_;                      ///< Number of noise octaves
    
    bool needsRegeneration_;                ///< Flag indicating if geometry needs regeneration
};
#pragma once

#include "FastNoiseLite.h"
#include <memory>

/**
 * @brief Wrapper class for FastNoiseLite providing easy-to-use noise generation
 * 
 * This class encapsulates FastNoiseLite functionality and provides convenient
 * methods for generating various types of noise commonly used in procedural generation.
 */
class Noise {
public:
    enum class NoiseType {
        OpenSimplex2,
        OpenSimplex2S,
        Cellular,
        Perlin,
        ValueCubic,
        Value
    };

    enum class FractalType {
        None,
        FBm,
        Ridged,
        PingPong,
        DomainWarpProgressive,
        DomainWarpIndependent
    };

    enum class CellularDistanceFunction {
        Euclidean,
        EuclideanSq,
        Manhattan,
        Hybrid
    };

    enum class CellularReturnType {
        CellValue,
        Distance,
        Distance2,
        Distance2Add,
        Distance2Sub,
        Distance2Mul,
        Distance2Div
    };

public:
    /**
     * @brief Construct a new Noise object with default settings
     * @param seed Random seed for noise generation
     */
    explicit Noise(int seed = 1337);

    /**
     * @brief Destroy the Noise object
     */
    ~Noise() = default;

    // Basic noise generation
    /**
     * @brief Generate 2D noise value
     * @param x X coordinate
     * @param y Y coordinate
     * @return float Noise value typically in range [-1, 1]
     */
    float get2D(float x, float y) const;

    /**
     * @brief Generate 3D noise value
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return float Noise value typically in range [-1, 1]
     */
    float get3D(float x, float y, float z) const;

    // Configuration methods
    /**
     * @brief Set the noise type
     * @param type Type of noise to generate
     */
    void setNoiseType(NoiseType type);

    /**
     * @brief Set the random seed
     * @param seed New seed value
     */
    void setSeed(int seed);

    /**
     * @brief Set the frequency of the noise
     * @param frequency Frequency value (higher = more detail)
     */
    void setFrequency(float frequency);

    // Fractal settings
    /**
     * @brief Set fractal type for layered noise
     * @param type Type of fractal
     */
    void setFractalType(FractalType type);

    /**
     * @brief Set number of fractal octaves
     * @param octaves Number of octaves (layers)
     */
    void setFractalOctaves(int octaves);

    /**
     * @brief Set fractal lacunarity
     * @param lacunarity Frequency multiplier between octaves
     */
    void setFractalLacunarity(float lacunarity);

    /**
     * @brief Set fractal gain
     * @param gain Amplitude multiplier between octaves
     */
    void setFractalGain(float gain);

    // Cellular noise settings
    /**
     * @brief Set cellular distance function
     * @param function Distance calculation method
     */
    void setCellularDistanceFunction(CellularDistanceFunction function);

    /**
     * @brief Set cellular return type
     * @param returnType What value to return from cellular noise
     */
    void setCellularReturnType(CellularReturnType returnType);

    /**
     * @brief Set cellular jitter
     * @param jitter Amount of randomness in cell positions
     */
    void setCellularJitter(float jitter);

    // Utility methods
    /**
     * @brief Get noise value normalized to [0, 1] range
     * @param x X coordinate
     * @param y Y coordinate
     * @return float Normalized noise value
     */
    float getNormalized2D(float x, float y) const;

    /**
     * @brief Get noise value normalized to [0, 1] range
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return float Normalized noise value
     */
    float getNormalized3D(float x, float y, float z) const;

    /**
     * @brief Generate fractal brownian motion (FBm) noise
     * @param x X coordinate
     * @param y Y coordinate
     * @param octaves Number of octaves
     * @param frequency Base frequency
     * @param amplitude Base amplitude
     * @param lacunarity Frequency multiplier
     * @param persistence Amplitude multiplier
     * @return float FBm noise value
     */
    float getFBm2D(float x, float y, int octaves = 4, float frequency = 0.01f, 
                   float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.5f);

    /**
     * @brief Generate fractal brownian motion (FBm) noise
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @param octaves Number of octaves
     * @param frequency Base frequency
     * @param amplitude Base amplitude
     * @param lacunarity Frequency multiplier
     * @param persistence Amplitude multiplier
     * @return float FBm noise value
     */
    float getFBm3D(float x, float y, float z, int octaves = 4, float frequency = 0.01f, 
                   float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.5f);

private:
    std::unique_ptr<FastNoiseLite> noise_;
};
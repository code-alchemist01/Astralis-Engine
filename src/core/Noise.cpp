#include "Noise.hpp"
#include <algorithm>

Noise::Noise(int seed) : noise_(std::make_unique<FastNoiseLite>()) {
    noise_->SetSeed(seed);
    noise_->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_->SetFrequency(0.01f);
    
    // Default fractal settings
    noise_->SetFractalType(FastNoiseLite::FractalType_FBm);
    noise_->SetFractalOctaves(4);
    noise_->SetFractalLacunarity(2.0f);
    noise_->SetFractalGain(0.5f);
    
    // Default cellular settings
    noise_->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    noise_->SetCellularJitter(1.0f);
}

float Noise::get2D(float x, float y) const {
    return noise_->GetNoise(x, y);
}

float Noise::get3D(float x, float y, float z) const {
    return noise_->GetNoise(x, y, z);
}

void Noise::setNoiseType(NoiseType type) {
    switch (type) {
        case NoiseType::OpenSimplex2:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            break;
        case NoiseType::OpenSimplex2S:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
            break;
        case NoiseType::Cellular:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_Cellular);
            break;
        case NoiseType::Perlin:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
            break;
        case NoiseType::ValueCubic:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);
            break;
        case NoiseType::Value:
            noise_->SetNoiseType(FastNoiseLite::NoiseType_Value);
            break;
    }
}

void Noise::setSeed(int seed) {
    noise_->SetSeed(seed);
}

void Noise::setFrequency(float frequency) {
    noise_->SetFrequency(frequency);
}

void Noise::setFractalType(FractalType type) {
    switch (type) {
        case FractalType::None:
            noise_->SetFractalType(FastNoiseLite::FractalType_None);
            break;
        case FractalType::FBm:
            noise_->SetFractalType(FastNoiseLite::FractalType_FBm);
            break;
        case FractalType::Ridged:
            noise_->SetFractalType(FastNoiseLite::FractalType_Ridged);
            break;
        case FractalType::PingPong:
            noise_->SetFractalType(FastNoiseLite::FractalType_PingPong);
            break;
        case FractalType::DomainWarpProgressive:
            noise_->SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
            break;
        case FractalType::DomainWarpIndependent:
            noise_->SetFractalType(FastNoiseLite::FractalType_DomainWarpIndependent);
            break;
    }
}

void Noise::setFractalOctaves(int octaves) {
    noise_->SetFractalOctaves(octaves);
}

void Noise::setFractalLacunarity(float lacunarity) {
    noise_->SetFractalLacunarity(lacunarity);
}

void Noise::setFractalGain(float gain) {
    noise_->SetFractalGain(gain);
}

void Noise::setCellularDistanceFunction(CellularDistanceFunction function) {
    switch (function) {
        case CellularDistanceFunction::Euclidean:
            noise_->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
            break;
        case CellularDistanceFunction::EuclideanSq:
            noise_->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
            break;
        case CellularDistanceFunction::Manhattan:
            noise_->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Manhattan);
            break;
        case CellularDistanceFunction::Hybrid:
            noise_->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
            break;
    }
}

void Noise::setCellularReturnType(CellularReturnType returnType) {
    switch (returnType) {
        case CellularReturnType::CellValue:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
            break;
        case CellularReturnType::Distance:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
            break;
        case CellularReturnType::Distance2:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
            break;
        case CellularReturnType::Distance2Add:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add);
            break;
        case CellularReturnType::Distance2Sub:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Sub);
            break;
        case CellularReturnType::Distance2Mul:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Mul);
            break;
        case CellularReturnType::Distance2Div:
            noise_->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
            break;
    }
}

void Noise::setCellularJitter(float jitter) {
    noise_->SetCellularJitter(jitter);
}

float Noise::getNormalized2D(float x, float y) const {
    return (get2D(x, y) + 1.0f) * 0.5f;
}

float Noise::getNormalized3D(float x, float y, float z) const {
    return (get3D(x, y, z) + 1.0f) * 0.5f;
}

float Noise::getFBm2D(float x, float y, int octaves, float frequency, 
                      float amplitude, float lacunarity, float persistence) {
    float result = 0.0f;
    float currentAmplitude = amplitude;
    float currentFrequency = frequency;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        result += get2D(x * currentFrequency, y * currentFrequency) * currentAmplitude;
        maxValue += currentAmplitude;
        
        currentAmplitude *= persistence;
        currentFrequency *= lacunarity;
    }
    
    return result / maxValue;
}

float Noise::getFBm3D(float x, float y, float z, int octaves, float frequency, 
                      float amplitude, float lacunarity, float persistence) {
    float result = 0.0f;
    float currentAmplitude = amplitude;
    float currentFrequency = frequency;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        result += get3D(x * currentFrequency, y * currentFrequency, z * currentFrequency) * currentAmplitude;
        maxValue += currentAmplitude;
        
        currentAmplitude *= persistence;
        currentFrequency *= lacunarity;
    }
    
    return result / maxValue;
}
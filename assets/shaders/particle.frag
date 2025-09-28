#version 330 core

// Inputs from vertex shader
in vec2 TexCoord;
in vec4 ParticleColor;
in float Life;
in vec3 WorldPos;
in vec3 ViewDir;
in float ParticleSize;

// Uniforms
uniform float time;
uniform int particleType; // 0: solar flare, 1: cosmic dust, 2: stellar wind, 3: corona
uniform vec3 viewPos;
uniform float globalIntensity;

out vec4 FragColor;

// Noise functions for procedural effects
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

float hash2(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash2(i);
    float b = hash2(i + vec2(1.0, 0.0));
    float c = hash2(i + vec2(0.0, 1.0));
    float d = hash2(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 3; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

// Calculate temperature-based color
vec3 calculateTemperatureColor(float temp) {
    // Temperature to color mapping (simplified blackbody radiation)
    if (temp < 3000.0) {
        return vec3(1.0, 0.3, 0.1); // Red
    } else if (temp < 5000.0) {
        return vec3(1.0, 0.7, 0.3); // Orange
    } else if (temp < 7000.0) {
        return vec3(1.0, 1.0, 0.8); // Yellow-white
    } else {
        return vec3(0.8, 0.9, 1.0); // Blue-white
    }
}

// Create circular particle shape with soft edges
float createParticleShape(vec2 uv) {
    float dist = length(uv - 0.5);
    return 1.0 - smoothstep(0.3, 0.5, dist);
}

// Create energy field effect
float createEnergyField(vec2 uv, float time) {
    vec2 p = uv * 4.0;
    float field = fbm(p + time * 0.5);
    field += fbm(p * 2.0 - time * 0.3) * 0.5;
    return field;
}

void main()
{
    vec2 uv = TexCoord;
    vec3 finalColor = ParticleColor.rgb;
    float finalAlpha = ParticleColor.a;
    
    // Create base particle shape
    float shape = createParticleShape(uv);
    
    // Apply particle type-specific effects
    if (particleType == 0) { // Solar flare
        // Intense, flickering energy
        float energy = createEnergyField(uv, time * 2.0);
        float flicker = 0.8 + 0.2 * sin(time * 10.0 + WorldPos.x);
        
        finalColor = mix(vec3(1.0, 0.3, 0.1), vec3(1.0, 0.8, 0.2), energy);
        finalColor *= (1.0 + energy * 0.5) * flicker;
        finalAlpha *= (1.0 - Life * 0.3); // Fade over time
        
    } else if (particleType == 1) { // Cosmic dust
        // Subtle, twinkling particles
        float twinkle = 0.7 + 0.3 * sin(time * 3.0 + WorldPos.y);
        float dustNoise = noise(uv * 8.0 + time * 0.1);
        
        finalColor = vec3(0.6, 0.7, 0.9) * (0.5 + dustNoise * 0.5);
        finalColor *= twinkle;
        finalAlpha *= (0.3 + Life * 0.4); // Grow brighter over time
        
    } else if (particleType == 2) { // Stellar wind
        // Fast-moving, streaky particles
        float speed = length(ViewDir);
        float streak = 1.0 + speed * 0.5;
        
        // Create elongated shape for motion blur
        vec2 stretchedUV = uv;
        stretchedUV.x *= (1.0 + speed * 0.3);
        shape = createParticleShape(stretchedUV);
        
        finalColor = vec3(0.8, 0.9, 1.0) * streak;
        finalAlpha *= (1.0 - Life * 0.7); // Quick fade
        
    } else if (particleType == 3) { // Corona
        // Glowing, pulsing corona particles
        float pulse = 0.6 + 0.4 * sin(time * 1.5 + WorldPos.z);
        float corona = createEnergyField(uv, time * 0.8);
        
        finalColor = calculateTemperatureColor(6000.0 + corona * 2000.0);
        finalColor *= pulse * (1.0 + corona * 0.3);
        finalAlpha *= (0.8 - Life * 0.2); // Slow fade
    }
    
    // Apply distance-based fading
    float distanceToCamera = length(viewPos - WorldPos);
    float distanceFade = 1.0 - smoothstep(500.0, 1000.0, distanceToCamera);
    
    // Apply global intensity
    finalColor *= globalIntensity;
    
    // Combine shape, color, and alpha
    finalAlpha *= shape * distanceFade;
    
    // Add subtle glow effect
    float glow = shape * 0.3;
    finalColor += glow;
    
    // Ensure we don't exceed maximum brightness
    finalColor = min(finalColor, vec3(2.0));
    
    FragColor = vec4(finalColor, finalAlpha);
}
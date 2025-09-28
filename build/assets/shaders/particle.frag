#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 ViewPos;

uniform vec3 particleColor;
uniform float alpha;
uniform float temperature;
uniform float intensity;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

out vec4 FragColor;

// Noise functions for procedural effects
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i.x + i.y * 57.0);
    float b = hash(i.x + 1.0 + i.y * 57.0);
    float c = hash(i.x + (i.y + 1.0) * 57.0);
    float d = hash(i.x + 1.0 + (i.y + 1.0) * 57.0);
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

// Calculate temperature-based color
vec3 calculateTemperatureColor(float temp) {
    temp = clamp(temp, 1000.0, 10000.0);
    float normalizedTemp = (temp - 1000.0) / 9000.0;
    
    vec3 color;
    if (normalizedTemp < 0.5) {
        // Red to yellow
        color.r = 1.0;
        color.g = normalizedTemp * 2.0;
        color.b = 0.0;
    } else {
        // Yellow to white to blue
        float t = (normalizedTemp - 0.5) * 2.0;
        color.r = 1.0;
        color.g = 1.0;
        color.b = t;
    }
    
    return color;
}

// Generate particle shape with soft edges
float generateParticleShape(vec2 uv) {
    float distance = length(uv - 0.5);
    
    // Add some noise for irregular shape
    float noiseValue = fbm(uv * 8.0) * 0.1;
    distance += noiseValue;
    
    // Soft circular falloff
    float circle = 1.0 - smoothstep(0.3, 0.5, distance);
    
    // Add some sparkle effect for high-temperature particles
    if (temperature > 5000.0) {
        float sparkle = fbm(uv * 16.0 + temperature * 0.001) * 0.3;
        circle += sparkle * (temperature / 10000.0);
    }
    
    return circle;
}

// Generate solar flare effects
vec3 generateFlareEffect(vec2 uv, float flareIntensity) {
    vec3 flareColor = vec3(1.0, 0.6, 0.2);
    
    // Create streaks
    float streak1 = abs(sin((uv.x - 0.5) * 20.0)) * 0.1;
    float streak2 = abs(sin((uv.y - 0.5) * 15.0)) * 0.1;
    
    // Add turbulence
    float turbulence = fbm(uv * 12.0 + flareIntensity) * 0.3;
    
    return flareColor * (streak1 + streak2 + turbulence) * flareIntensity;
}

// Generate cosmic dust effects
vec3 generateDustEffect(vec2 uv) {
    vec3 dustColor = vec3(0.6, 0.5, 0.4);
    
    // Create dust grain texture
    float grain = fbm(uv * 32.0) * 0.5;
    float density = fbm(uv * 8.0) * 0.3;
    
    return dustColor * (grain + density);
}

// Generate stellar wind effects
vec3 generateWindEffect(vec2 uv, float windStrength) {
    vec3 windColor = vec3(0.8, 0.9, 1.0);
    
    // Create flowing patterns
    float flow = sin(uv.x * 10.0 + windStrength * 5.0) * 0.2;
    float turbulence = fbm(uv * 6.0 + windStrength) * 0.4;
    
    return windColor * (flow + turbulence) * windStrength;
}

void main() {
    vec2 uv = TexCoord;
    
    // Generate base particle shape
    float particleShape = generateParticleShape(uv);
    
    if (particleShape < 0.01) {
        discard;
    }
    
    // Start with base particle color
    vec3 finalColor = particleColor;
    
    // Apply temperature-based coloring
    if (temperature > 2000.0) {
        vec3 tempColor = calculateTemperatureColor(temperature);
        finalColor = mix(finalColor, tempColor, 0.7);
    }
    
    // Add type-specific effects based on intensity
    if (intensity > 0.5) {
        // Solar flare effects
        vec3 flareEffect = generateFlareEffect(uv, intensity);
        finalColor += flareEffect;
    } else if (temperature < 1000.0) {
        // Cosmic dust effects
        vec3 dustEffect = generateDustEffect(uv);
        finalColor = mix(finalColor, dustEffect, 0.6);
    } else if (temperature > 100000.0) {
        // Stellar wind effects
        vec3 windEffect = generateWindEffect(uv, intensity);
        finalColor += windEffect * 0.5;
    }
    
    // Calculate distance-based lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.01 * distance + 0.001 * distance * distance);
    
    // Simple lighting
    float ambient = 0.3;
    float diffuse = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
    vec3 lighting = (ambient + diffuse * attenuation) * lightColor;
    
    finalColor *= lighting;
    
    // Add bloom effect for bright particles
    if (temperature > 5000.0 || intensity > 0.8) {
        float bloom = (temperature / 10000.0 + intensity) * 0.3;
        finalColor += finalColor * bloom;
    }
    
    // Apply particle shape and alpha
    float finalAlpha = particleShape * alpha;
    
    // Distance fade
    float viewDistance = length(viewPos - FragPos);
    float distanceFade = 1.0 / (1.0 + viewDistance * 0.001);
    finalAlpha *= distanceFade;
    
    FragColor = vec4(finalColor, finalAlpha);
}
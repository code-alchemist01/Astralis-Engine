#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 WorldPos;

out vec4 FragColor;

uniform vec3 sunColor;
uniform float sunIntensity;
uniform float sunTemperature;
uniform float pulsePhase;
uniform vec3 viewPos;

// Noise functions for surface activity
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

// Generate solar surface activity
vec3 generateSolarSurface(vec2 uv) {
    // Animated surface patterns
    float time = pulsePhase * 0.5;
    vec2 p = uv * 8.0 + time * 0.1;
    
    // Solar flares and granulation
    float granulation = fbm(p * 4.0);
    float flares = fbm(p * 2.0 + time * 0.2);
    float spots = fbm(p * 1.0 + time * 0.05);
    
    // Combine patterns
    float activity = granulation * 0.4 + flares * 0.4 + spots * 0.2;
    
    // Create color variations based on activity
    vec3 hotColor = sunColor * 1.2;
    vec3 coolColor = sunColor * 0.8;
    vec3 surfaceColor = mix(coolColor, hotColor, activity);
    
    return surfaceColor;
}

// Calculate corona/glow effect
float calculateCorona(vec3 normal, vec3 viewDir) {
    float fresnel = 1.0 - max(dot(normal, viewDir), 0.0);
    fresnel = pow(fresnel, 1.5);
    
    // Add pulsing to corona
    float pulse = 1.0 + 0.2 * sin(pulsePhase * 2.0);
    
    return fresnel * pulse;
}

void main()
{
    // Generate dynamic solar surface
    vec3 surfaceColor = generateSolarSurface(TexCoord);
    
    // Calculate view direction
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 normal = normalize(Normal);
    
    // Base emission (sun is self-illuminating)
    vec3 emission = surfaceColor * sunIntensity;
    
    // Add pulsing effect
    float pulse = 1.0 + 0.1 * sin(pulsePhase * 3.0);
    emission *= pulse;
    
    // Calculate corona glow
    float corona = calculateCorona(normal, viewDir);
    vec3 coronaColor = sunColor * corona * 0.5;
    
    // Combine surface and corona
    vec3 finalColor = emission + coronaColor;
    
    // Add temperature-based intensity boost
    float tempFactor = sunTemperature / 5778.0; // Normalize to Sun's temperature
    finalColor *= (0.8 + 0.4 * tempFactor);
    
    // Add subtle center brightening
    float centerDist = length(TexCoord - vec2(0.5));
    float centerBright = 1.0 + 0.3 * (1.0 - smoothstep(0.0, 0.5, centerDist));
    finalColor *= centerBright;
    
    FragColor = vec4(finalColor, 1.0);
}
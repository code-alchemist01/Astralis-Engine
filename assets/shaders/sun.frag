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
uniform float solarFlareIntensity;
uniform float currentLightIntensity;

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

// Generate solar flares
vec3 generateSolarFlares(vec2 uv, float flareIntensity) {
    vec2 p = uv * 6.0 + pulsePhase * 0.3;
    
    // Create flare patterns
    float flare1 = fbm(p + vec2(sin(pulsePhase * 1.3), cos(pulsePhase * 0.7)));
    float flare2 = fbm(p * 1.5 + vec2(cos(pulsePhase * 0.9), sin(pulsePhase * 1.1)));
    
    // Combine flares with intensity
    float flarePattern = (flare1 + flare2) * 0.5;
    flarePattern = pow(flarePattern, 2.0 - flareIntensity); // More intense = more visible flares
    
    // Flare colors - bright white/yellow
    vec3 flareColor = vec3(1.5, 1.2, 0.8) * flareIntensity;
    
    return flareColor * flarePattern;
}

// Generate solar surface activity
vec3 generateSolarSurface(vec2 uv) {
    // Animated surface patterns
    float time = pulsePhase * 0.5;
    vec2 p = uv * 8.0 + time * 0.1;
    
    // Solar granulation and convection cells
    float granulation = fbm(p * 4.0);
    float convection = fbm(p * 2.0 + time * 0.2);
    float spots = fbm(p * 1.0 + time * 0.05);
    
    // Combine patterns
    float activity = granulation * 0.4 + convection * 0.4 + spots * 0.2;
    
    // Create more vibrant yellow/orange color variations
    vec3 baseYellow = vec3(1.0, 0.8, 0.3);  // Strong yellow-orange base
    vec3 hotColor = sunColor * baseYellow * 1.5;  // Brighter hot areas
    vec3 coolColor = sunColor * baseYellow * 0.7;  // Darker cool areas
    vec3 surfaceColor = mix(coolColor, hotColor, activity);
    
    // Add solar flares
    vec3 flares = generateSolarFlares(uv, solarFlareIntensity);
    surfaceColor += flares;
    
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
    vec3 emission = surfaceColor * currentLightIntensity;
    
    // Add pulsing effect with solar activity
    float pulse = 1.0 + 0.1 * sin(pulsePhase * 3.0);
    float solarActivity = 1.0 + solarFlareIntensity * 0.3; // Solar flares boost emission
    emission *= pulse * solarActivity;
    
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
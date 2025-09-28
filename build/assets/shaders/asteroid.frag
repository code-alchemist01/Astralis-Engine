#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 planetColor;
uniform float planetSeed;

// Noise functions for procedural textures
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

vec3 generateNormalMap(vec2 uv, float strength) {
    float offset = 0.001;
    float heightL = fbm(uv - vec2(offset, 0.0));
    float heightR = fbm(uv + vec2(offset, 0.0));
    float heightD = fbm(uv - vec2(0.0, offset));
    float heightU = fbm(uv + vec2(0.0, offset));
    
    vec3 normal = vec3((heightL - heightR) * strength, (heightD - heightU) * strength, 1.0);
    return normalize(normal);
}

vec3 generateAsteroidTexture(vec2 uv) {
    vec2 seedOffset = vec2(planetSeed * 0.1, planetSeed * 0.13);
    vec2 noiseUV = uv * 8.0 + seedOffset;
    
    // Base rocky texture
    float rockNoise = fbm(noiseUV);
    float detailNoise = fbm(noiseUV * 4.0) * 0.3;
    float crackNoise = fbm(noiseUV * 16.0) * 0.1;
    
    // Combine noises
    float combined = rockNoise + detailNoise + crackNoise;
    
    // Create color variations
    vec3 darkRock = vec3(0.2, 0.15, 0.1);
    vec3 lightRock = vec3(0.4, 0.35, 0.25);
    vec3 metallic = vec3(0.3, 0.3, 0.35);
    
    // Mix colors based on noise
    vec3 baseColor = mix(darkRock, lightRock, smoothstep(0.3, 0.7, combined));
    
    // Add metallic veins
    float metallicNoise = fbm(noiseUV * 12.0 + vec2(100.0));
    if (metallicNoise > 0.8) {
        baseColor = mix(baseColor, metallic, (metallicNoise - 0.8) * 5.0);
    }
    
    // Apply planet color tint
    baseColor *= planetColor;
    
    return baseColor;
}

void main() {
    vec2 uv = TexCoord;
    
    // Generate procedural texture
    vec3 albedo = generateAsteroidTexture(uv);
    
    // Generate normal map
    vec3 normal = generateNormalMap(uv, 2.0);
    
    // Lighting calculations in tangent space
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Ambient lighting
    vec3 ambient = 0.15 * albedo;
    
    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * albedo;
    
    // Specular lighting (low for rocky surfaces)
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular = spec * lightColor * 0.1;
    
    // Distance-based attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance + 0.000001 * distance * distance);
    
    // Combine lighting
    vec3 result = ambient + (diffuse + specular) * attenuation;
    
    // Add slight rim lighting for better visibility
    vec3 worldNormal = normalize(Normal);
    vec3 worldViewDir = normalize(viewPos - FragPos);
    float rim = 1.0 - max(dot(worldNormal, worldViewDir), 0.0);
    rim = pow(rim, 3.0);
    result += rim * lightColor * 0.1;
    
    FragColor = vec4(result, 1.0);
}
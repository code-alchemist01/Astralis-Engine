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
uniform int planetType; // 0=rocky, 1=gas, 2=ice, 3=desert
uniform float lightIntensity;

// Noise functions for procedural textures
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
    
    for(int i = 0; i < 6; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

// Generate procedural normal map
vec3 generateNormalMap(vec2 uv, float seed, int type) {
    vec2 p = uv * 16.0 + seed;
    float offset = 0.001;
    
    // Sample height at current position and neighbors
    float h = fbm(p);
    float hx = fbm(p + vec2(offset, 0.0));
    float hy = fbm(p + vec2(0.0, offset));
    
    // Calculate normal from height differences
    vec3 normal;
    normal.x = (h - hx) / offset;
    normal.y = (h - hy) / offset;
    normal.z = 1.0;
    
    // Adjust normal strength based on planet type
    float strength = 1.0;
    if(type == 0) strength = 2.0;      // Rocky - strong normals
    else if(type == 1) strength = 0.5; // Gas - smooth normals
    else if(type == 2) strength = 1.5; // Ice - medium normals
    else strength = 1.8;               // Desert - strong normals
    
    normal.xy *= strength;
    return normalize(normal);
}

// Calculate atmospheric glow effect
vec3 calculateAtmosphere(vec3 normal, vec3 viewDir, int type) {
    float fresnel = 1.0 - max(dot(normal, viewDir), 0.0);
    fresnel = pow(fresnel, 2.0);
    
    vec3 atmosphereColor;
    float intensity;
    
    if(type == 0) { // Rocky planet - blue atmosphere
        atmosphereColor = vec3(0.4, 0.7, 1.0);
        intensity = 0.3;
    }
    else if(type == 1) { // Gas giant - colorful atmosphere
        atmosphereColor = vec3(1.0, 0.8, 0.4);
        intensity = 0.5;
    }
    else if(type == 2) { // Ice planet - pale blue atmosphere
        atmosphereColor = vec3(0.8, 0.9, 1.0);
        intensity = 0.2;
    }
    else { // Desert planet - orange atmosphere
        atmosphereColor = vec3(1.0, 0.6, 0.3);
        intensity = 0.25;
    }
    
    return atmosphereColor * fresnel * intensity;
}

vec3 generatePlanetTexture(vec2 uv, float seed, int type) {
    vec2 p = uv * 8.0 + seed;
    
    if(type == 0) { // Rocky planet
        float continents = fbm(p * 0.5);
        float mountains = fbm(p * 2.0) * 0.3;
        float detail = fbm(p * 8.0) * 0.1;
        
        float height = continents + mountains + detail;
        
        vec3 ocean = vec3(0.1, 0.3, 0.8);
        vec3 land = vec3(0.4, 0.6, 0.2);
        vec3 mountain = vec3(0.6, 0.5, 0.4);
        vec3 snow = vec3(0.9, 0.9, 0.95);
        
        vec3 color = mix(ocean, land, smoothstep(0.3, 0.4, height));
        color = mix(color, mountain, smoothstep(0.6, 0.7, height));
        color = mix(color, snow, smoothstep(0.8, 0.9, height));
        
        return color;
    }
    else if(type == 1) { // Gas giant
        float bands = sin(uv.y * 20.0 + fbm(p) * 2.0) * 0.5 + 0.5;
        float storms = fbm(p * 3.0);
        
        vec3 color1 = vec3(0.8, 0.6, 0.3);
        vec3 color2 = vec3(0.9, 0.7, 0.4);
        vec3 storm = vec3(0.9, 0.4, 0.2);
        
        vec3 color = mix(color1, color2, bands);
        color = mix(color, storm, smoothstep(0.7, 0.8, storms));
        
        return color;
    }
    else if(type == 2) { // Ice planet
        float cracks = fbm(p * 4.0);
        float ice = fbm(p * 1.0);
        
        vec3 ice_color = vec3(0.8, 0.9, 1.0);
        vec3 deep_ice = vec3(0.6, 0.8, 0.9);
        vec3 crack_color = vec3(0.3, 0.4, 0.6);
        
        vec3 color = mix(deep_ice, ice_color, ice);
        color = mix(color, crack_color, smoothstep(0.6, 0.7, cracks));
        
        return color;
    }
    else { // Desert planet
        float dunes = fbm(p * 2.0);
        float rocks = fbm(p * 6.0);
        
        vec3 sand = vec3(0.8, 0.7, 0.4);
        vec3 dark_sand = vec3(0.6, 0.5, 0.3);
        vec3 rock = vec3(0.5, 0.4, 0.3);
        
        vec3 color = mix(dark_sand, sand, dunes);
        color = mix(color, rock, smoothstep(0.7, 0.8, rocks));
        
        return color;
    }
}

void main()
{
    // Generate procedural planet texture and normal map
    vec3 surfaceColor = generatePlanetTexture(TexCoord, planetSeed, planetType);
    vec3 normalMap = generateNormalMap(TexCoord, planetSeed, planetType);
    
    // Use normal mapping for enhanced surface detail
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    
    // Calculate distance attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance + 0.000001 * distance * distance);
    
    // Ambient lighting with dynamic intensity
    float ambientStrength = 0.15 + 0.1 * lightIntensity; // Varies with solar activity
    vec3 ambient = ambientStrength * lightColor * attenuation;
    
    // Diffuse lighting with normal mapping and dynamic intensity
    float diff = max(dot(normalMap, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity * attenuation;
    
    // Specular lighting with normal mapping and dynamic intensity
    float specularStrength = 0.15;
    vec3 reflectDir = reflect(-lightDir, normalMap);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * lightIntensity * attenuation;
    
    // Calculate atmospheric glow
    vec3 worldNormal = normalize(Normal);
    vec3 worldViewDir = normalize(viewPos - FragPos);
    vec3 atmosphere = calculateAtmosphere(worldNormal, worldViewDir, planetType);
    
    // Combine all lighting effects
    vec3 lighting = ambient + diffuse + specular;
    vec3 result = lighting * surfaceColor + atmosphere;
    
    // Add subtle rim lighting for depth
    float rim = 1.0 - max(dot(worldNormal, worldViewDir), 0.0);
    rim = pow(rim, 3.0);
    result += rim * lightColor * 0.1;
    
    FragColor = vec4(result, 1.0);
}
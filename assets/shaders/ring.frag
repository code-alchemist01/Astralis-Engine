#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 planetColor;
uniform float alpha;

// Noise function for particle texture
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

void main() {
    vec2 uv = TexCoord;
    
    // Create circular particle shape
    vec2 center = vec2(0.5);
    float dist = distance(uv, center);
    
    // Soft circular falloff
    float circle = 1.0 - smoothstep(0.3, 0.5, dist);
    
    if (circle < 0.01) {
        discard;
    }
    
    // Add some noise for irregular particle shape
    float noiseValue = noise(uv * 8.0);
    circle *= (0.7 + 0.3 * noiseValue);
    
    // Basic lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Simple diffuse lighting
    float diff = max(dot(Normal, lightDir), 0.0);
    
    // Distance-based attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance + 0.000001 * distance * distance);
    
    // Combine lighting with particle color
    vec3 ambient = 0.2 * planetColor;
    vec3 diffuse = diff * lightColor * planetColor * attenuation;
    vec3 result = ambient + diffuse;
    
    // Add slight glow effect
    float glow = pow(circle, 0.5);
    result += glow * planetColor * 0.3;
    
    // Final alpha combines circle shape, transparency, and distance fade
    float finalAlpha = circle * alpha;
    
    FragColor = vec4(result, finalAlpha);
}
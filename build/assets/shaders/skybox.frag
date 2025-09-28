#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube uSkybox;
uniform bool uUseStarfield;
uniform float uStarDensity;
uniform float uStarBrightness;
uniform uint uSeed;

// Hash function for procedural star generation
float hash(vec3 p) {
    p = fract(p * vec3(443.8975, 397.2973, 491.1871));
    p += dot(p.zxy, p.yxz + 19.27);
    return fract(p.x * p.y * p.z);
}

// Generate stars based on direction
vec3 generateStars(vec3 dir) {
    // Normalize direction and scale for star grid
    vec3 starCoord = normalize(dir) * 50.0;
    
    // Create a grid for star placement
    vec3 gridPos = floor(starCoord);
    vec3 fracPos = fract(starCoord);
    
    float starIntensity = 0.0;
    
    // Check current cell and neighbors for stars
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            for(int z = -1; z <= 1; z++) {
                vec3 cellPos = gridPos + vec3(x, y, z);
                
                // Add seed to make stars deterministic but varied
                vec3 seedPos = cellPos + vec3(uSeed * 0.001);
                
                // Generate random values for this cell
                float h1 = hash(seedPos);
                float h2 = hash(seedPos + vec3(127.1, 311.7, 74.7));
                float h3 = hash(seedPos + vec3(269.5, 183.3, 246.1));
                
                // Star probability based on density (make it more likely)
                if(h1 < uStarDensity * 10.0) {
                    // Star position within cell
                    vec3 starPos = vec3(x, y, z) + vec3(h1, h2, h3);
                    vec3 toStar = starPos - fracPos;
                    float dist = length(toStar);
                    
                    // Larger star size for visibility
                    float starSize = 0.1 + hash(seedPos + vec3(456.789)) * 0.1;
                    float brightness = 0.8 + hash(seedPos + vec3(789.123)) * 0.2;
                    
                    // Create star with falloff
                    if(dist < starSize) {
                        float falloff = 1.0 - (dist / starSize);
                        starIntensity += brightness * falloff * falloff;
                    }
                }
            }
        }
    }
    
    return vec3(starIntensity * uStarBrightness);
}

void main() {
    vec3 color;
    
    if(uUseStarfield) {
        // Generate procedural starfield
        vec3 stars = generateStars(TexCoords);
        
        // Create a dark space background with slight color variation
        vec3 spaceColor = vec3(0.01, 0.01, 0.02) + 
                         vec3(hash(TexCoords * 0.1)) * 0.02;
        
        color = spaceColor + stars;
    } else {
        // Use cubemap texture
        color = texture(uSkybox, TexCoords).rgb;
    }
    
    FragColor = vec4(color, 1.0);
}
#version 330 core

// Vertex attributes (quad vertices)
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

// Instance attributes (per particle)
layout (location = 2) in vec3 aInstancePos;     // Particle position
layout (location = 3) in vec3 aInstanceVel;     // Particle velocity
layout (location = 4) in vec4 aInstanceColor;   // Particle color (RGB + alpha)
layout (location = 5) in vec2 aInstanceData;    // x: size, y: life

// Uniforms
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float time;
uniform int particleType; // 0: solar flare, 1: cosmic dust, 2: stellar wind, 3: corona

// Outputs to fragment shader
out vec2 TexCoord;
out vec4 ParticleColor;
out float Life;
out vec3 WorldPos;
out vec3 ViewDir;
out float ParticleSize;

void main()
{
    // Get particle properties
    vec3 particlePos = aInstancePos;
    vec3 particleVel = aInstanceVel;
    vec4 particleColor = aInstanceColor;
    float particleSize = aInstanceData.x;
    float particleLife = aInstanceData.y;
    
    // Calculate billboard orientation (face camera)
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    // Apply size scaling based on particle type and life
    float sizeMultiplier = 1.0;
    if (particleType == 0) { // Solar flare
        sizeMultiplier = 1.0 + sin(time * 3.0 + particlePos.x) * 0.3;
        sizeMultiplier *= (1.0 - particleLife * 0.5); // Shrink over time
    } else if (particleType == 1) { // Cosmic dust
        sizeMultiplier = 0.5 + particleLife * 0.5; // Grow over time
    } else if (particleType == 2) { // Stellar wind
        sizeMultiplier = 1.0 - particleLife * 0.8; // Fade and shrink
    } else if (particleType == 3) { // Corona
        sizeMultiplier = 1.0 + sin(time * 2.0 + particlePos.y) * 0.4;
    }
    
    float finalSize = particleSize * sizeMultiplier;
    
    // Create billboard quad
    vec3 worldPos = particlePos + 
                   cameraRight * aPos.x * finalSize + 
                   cameraUp * aPos.y * finalSize;
    
    // Apply some movement based on velocity for trailing effect
    if (particleType == 0 || particleType == 2) { // Solar flare or stellar wind
        worldPos += particleVel * aPos.z * 0.1; // Use Z for trailing
    }
    
    // Transform to clip space
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    // Pass data to fragment shader
    TexCoord = aTexCoord;
    ParticleColor = particleColor;
    Life = particleLife;
    WorldPos = worldPos;
    ViewDir = normalize(viewPos - worldPos);
    ParticleSize = finalSize;
}
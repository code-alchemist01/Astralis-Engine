#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    
    // Calculate tangent and bitangent for normal mapping
    vec3 T = normalize(vec3(model * vec4(cross(aNormal, vec3(0.0, 1.0, 0.0)), 0.0)));
    vec3 N = normalize(Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    // Create TBN matrix for tangent space
    mat3 TBN = transpose(mat3(T, B, N));
    
    // Transform light and view positions to tangent space
    TangentLightPos = TBN * lightPos;
    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * FragPos;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
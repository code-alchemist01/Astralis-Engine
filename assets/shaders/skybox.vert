#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    TexCoords = aPos;
    
    // Remove translation from view matrix (only rotation)
    mat4 rotView = mat4(mat3(uView));
    vec4 pos = uProjection * rotView * vec4(aPos, 1.0);
    
    // Set z to w so that z/w = 1.0 (maximum depth)
    gl_Position = pos.xyww;
}
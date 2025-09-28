#version 330 core

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec3 color;
uniform float alpha;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);
    FragColor = vec4(texColor.rgb * color, texColor.a * alpha);
}
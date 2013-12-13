#version 330 core

in vec3 texCoord;

out vec4 color;

uniform samplerCube cubemapSampler;

void main() {
    color = texture(cubemapSampler, texCoord);
}

#version 330 core

in vec3 texCoord;

out vec4 color;

uniform samplerCube cubemapSampler;
uniform int wireframeMode;

void main() {
    if(wireframeMode == 1) {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        color = texture(cubemapSampler, texCoord);
    }
}

#version 330 core

in vec2 texCoord;

out vec4 color;

uniform sampler2D texSampler;
uniform int wireframeMode;

void main() {
    if(wireframeMode == 1) {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        color = texture(texSampler, texCoord);
    }
}

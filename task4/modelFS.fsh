#version 330 core

out vec4 color;

uniform int wireframeMode;
uniform vec3 modelColor;

void main() {
    if(wireframeMode == 1) {
        color = vec4(modelColor, 1.0);
    } else {
        color = vec4(modelColor, 0.3);
    }
}

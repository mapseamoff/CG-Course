#version 330 core

in vec2 texCoord;
in vec3 ptcPos;
out vec4 color;

uniform vec3 cameraPos;
uniform sampler2D texSampler;
uniform float maxDist;
uniform float cubeSize;
uniform int wireframeMode;

void main() {
    if(wireframeMode == 0) {
        color.rgb = texture(texSampler, texCoord).rgb;
        float halfSize = cubeSize / 2;
        float dist = length(ptcPos - cameraPos);

        if(dist <= maxDist) {
            color.a = 1.0;
        } else if(dist > halfSize) {
            color.a = 0.0;
        } else {
            color.a = (halfSize - dist) / (halfSize - maxDist);
        }

        if(color.r < 0.15 && color.g < 0.15 && color.b < 0.15) discard;
    } else {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
}

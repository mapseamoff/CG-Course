#version 330 core

in vec2 texCoord;
in vec3 vertexNormal;

out vec4 color;

uniform sampler2D texSampler;
uniform int wireframeMode;
uniform int textureMode;
uniform float userContrast;

void main() {
    if(wireframeMode == 1) {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        if(textureMode == 0) {
            vec3 texColor = texture(texSampler, texCoord).rgb;
            vec3 normColor = vec3(abs(vertexNormal.z), abs(vertexNormal.z), abs(vertexNormal.z));
            vec3 tmpColor = texColor * normColor;
            tmpColor = (tmpColor - 0.5f) * userContrast + 0.5f;
            color = vec4(tmpColor, 1.0);
//            color = vec4(texColor * normColor, 1.0);
        }
        else if(textureMode == 1) color = texture(texSampler, texCoord);
        else if(textureMode == 2) color = vec4(vertexNormal, 1.0);
    }
}

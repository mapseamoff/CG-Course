#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 MVP;

out vec3 texCoord;

void main() {
    vec4 pos = MVP * vec4(vertexPosition_modelspace, 1.0);
    gl_Position = pos.xyww;
    texCoord = normalize(vertexPosition_modelspace);
}

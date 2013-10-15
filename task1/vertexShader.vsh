#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

out float zCoord;
uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
    zCoord = gl_Position.z;
}

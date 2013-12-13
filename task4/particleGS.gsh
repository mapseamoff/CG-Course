#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 pass_xyzs[];
out vec2 texCoord;
out vec3 ptcPos;

uniform mat4 VP;
uniform vec3 cameraPos;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec2 viewportSize;
uniform int billboardType;

void main() {
    float particleSize = pass_xyzs[0].w;
    if(billboardType == 0) particleSize /= 10.0;

    vec3 vertexPos = gl_in[0].gl_Position.xyz;
    vec3 camr = 0.5 * particleSize * cameraRight;
    vec3 camu = 0.5 * particleSize * cameraUp;

    vec4 vpos = VP * vec4(vertexPos, 1.0);
    float dx = particleSize * 0.5 * vpos.w / viewportSize.x;
    float dy = particleSize * 0.5 * vpos.w / viewportSize.y;

    if(billboardType == 0) {
//        vertexPos -= (cameraRight * 0.5 * particleSize);
//        gl_Position = VP * vec4(vertexPos, 1.0);
        gl_Position = VP * vec4(vertexPos - camu - camr, 1.0);
    } else {
        gl_Position = vec4(vpos.x - dx, vpos.y - dy, vpos.z, vpos.w);
    }
    texCoord = vec2(0.0, 0.0);
    ptcPos = vertexPos;
    EmitVertex();

    if(billboardType == 0) {
//        vertexPos.y += particleSize;
//        gl_Position = VP * vec4(vertexPos, 1.0);
        gl_Position = VP * vec4(vertexPos - camu + camr, 1.0);
    } else {
        gl_Position = vec4(vpos.x - dx, vpos.y + dy, vpos.z, vpos.w);
    }
    texCoord = vec2(0.0, 1.0);
    ptcPos = vertexPos;
    EmitVertex();

    if(billboardType == 0) {
//        vertexPos.y -= particleSize;
//        vertexPos += cameraRight * particleSize;
//        gl_Position = VP * vec4(vertexPos, 1.0);
        gl_Position = VP * vec4(vertexPos + camu - camr, 1.0);
    } else {
        gl_Position = vec4(vpos.x + dx, vpos.y - dy, vpos.z, vpos.w);
    }
    texCoord = vec2(1.0, 0.0);
    ptcPos = vertexPos;
    EmitVertex();

    if(billboardType == 0) {
//        vertexPos.y += particleSize;
//        gl_Position = VP * vec4(vertexPos, 1.0);
        gl_Position = VP * vec4(vertexPos + camu + camr, 1.0);
    } else {
        gl_Position = vec4(vpos.x + dx, vpos.y + dy, vpos.z, vpos.w);
    }
    texCoord = vec2(1.0, 1.0);
    ptcPos = vertexPos;
    EmitVertex();

    EndPrimitive();
}

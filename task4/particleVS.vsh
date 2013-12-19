#version 330 core

layout(location = 0) in vec4 xyzs;   //position and size
layout(location = 1) in vec3 srf;    //speed, radius, frequence
layout(location = 2) in float dl;    //delay

uniform mat4 VP;
uniform vec3 cameraPos;
uniform float cubeSize;
uniform int time;
uniform vec3 constantShift;

out vec4 pass_xyzs;
out int skipVertex;

//-------------------------------------------------------------------------------------

float adjustPos(float camPos, float vertPos, float size) {
    float halfSize = size / 2.0;
    if(camPos > 0 && vertPos <= -halfSize + camPos) return vertPos + size;
    else if(camPos < 0 && vertPos >= halfSize + camPos) return vertPos - size;
    return vertPos;
}

vec3 adjustPos(vec3 camPos, vec3 vertPos, float size) {
    vec3 res;
    res.x = adjustPos(camPos.x, vertPos.x, size);
    res.y = adjustPos(camPos.y, vertPos.y, size);
    res.z = adjustPos(camPos.z, vertPos.z, size);
    return res;
}

//-------------------------------------------------------------------------------------

float getCubeIndex(float pos, float size) {
    return sign(pos) * floor((abs(pos) + (size / 2.0)) / size);
}

vec3 getCubeIndex(vec3 pos, float size) {
    return vec3(getCubeIndex(pos.x, size), getCubeIndex(pos.y, size), getCubeIndex(pos.z, size));
}

//-------------------------------------------------------------------------------------

void main() {
    vec3 vpos = xyzs.xyz;

    if(time - dl >= 0) {
        float lifeDist = cubeSize / 2.0;
        float curDist = srf.x * time;
        float modDist = curDist - lifeDist * floor(curDist / lifeDist);

        vpos.y = xyzs.y - modDist;
        vpos.x = xyzs.x + srf.y * cos(srf.z * time);
        vpos.z = xyzs.z + srf.y * sin(srf.z * time);

        //should be cubeSize for whole particle system or 0.5*cubeSize for octant
        float realSize = 0.5 * cubeSize;
        //adjust particle pos
        vec3 cubeIndex = getCubeIndex(cameraPos, realSize);
        vec3 cubeCameraPos = cameraPos - realSize * cubeIndex;
        vpos = adjustPos(cubeCameraPos, vpos, realSize);

        //move all particles to the current camera cube
        vpos += realSize * cubeIndex + constantShift;

        gl_Position = vec4(vpos, 1.0);
        skipVertex = 0;
    } else {
        gl_Position = vec4(xyzs.xyz, 1.0);
        skipVertex = 1;
    }

    pass_xyzs = vec4(vpos, xyzs.w);
}

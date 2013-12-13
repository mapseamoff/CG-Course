#version 330 core

layout(location = 0) in vec4 xyzs;   //position and size
layout(location = 1) in vec3 srf;    //speed, radius, frequence

uniform mat4 VP;
uniform vec3 cameraPos;
uniform float cubeSize;
uniform int time;

out vec4 pass_xyzs;

//-------------------------------------------------------------------------------------

float adjustPos(float camPos, float vertPos) {
    float halfSize = cubeSize / 2.0;
    if(camPos > 0 && vertPos <= -halfSize + camPos) return vertPos + cubeSize;
    else if(camPos < 0 && vertPos >= halfSize + camPos) return vertPos - cubeSize;
    return vertPos;
}

vec3 adjustPos(vec3 camPos, vec3 vertPos) {
    vec3 res;
    res.x = adjustPos(camPos.x, vertPos.x);
    res.y = adjustPos(camPos.y, vertPos.y);
    res.z = adjustPos(camPos.z, vertPos.z);
    return res;
}

//-------------------------------------------------------------------------------------

float getCubeIndex(float pos) {
    return sign(pos) * floor((abs(pos) + (cubeSize / 2.0)) / cubeSize);
}

vec3 getCubeIndex(vec3 pos) {
    return vec3(getCubeIndex(pos.x), getCubeIndex(pos.y), getCubeIndex(pos.z));
}

//-------------------------------------------------------------------------------------

void main() {
    vec3 vpos = xyzs.xyz;

    float lifeDistance = xyzs.y + cubeSize / 2.0;
    int timeMod = int(ceil(lifeDistance / srf.x));
    vpos.y = xyzs.y - srf.x * (time % timeMod);
    vpos.x = xyzs.x + srf.y * cos(srf.z * time);
    vpos.z = xyzs.z + srf.y * sin(srf.z * time);

//    float cubeX = sign(cameraPos.x) * floor((abs(cameraPos.x) + halfSize) / cubeSize);
//    float cubeY = sign(cameraPos.y) * floor((abs(cameraPos.y) + halfSize) / cubeSize);
//    float cubeZ = sign(cameraPos.z) * floor((abs(cameraPos.z) + halfSize) / cubeSize);
    vec3 cubeIndex = getCubeIndex(cameraPos);
    vec3 cubeCameraPos = cameraPos - cubeSize * cubeIndex;

    //adjust particle pos
    vpos = adjustPos(cubeCameraPos, vpos);

    //move all particles to current camera cube
    vpos += cubeSize * cubeIndex;

    gl_Position = vec4(vpos, 1.0);
    pass_xyzs = vec4(vpos, xyzs.w);

}

#version 330 core

layout(location = 0) in vec4 xyzs;   //position and size
layout(location = 1) in vec3 srf;    //speed, radius, frequence, delay

uniform mat4 VP;
uniform vec3 cameraPos;
uniform float cubeSize;
uniform int time;
uniform int octants;

out vec4 pass_xyzs;
out int skipVertex;

//-------------------------------------------------------------------------------------

int checkBit(int n, int bit) {
    if((n & (1 << bit)) > 0) return 1;
    return 0;
}

int getOctant(vec3 pos) {
    //y < 0 - 4 5 6 7 100 101 110 111 - bit 2
    //y > 0 - 0 1 2 3
    //x < 0 - 0 2 4 6 000 010 100 110 - bit 0
    //x > 0 - 1 3 5 7 001 011 101 111
    //z < 0 - 0 1 4 5 000 001 100 101
    //z > 0 - 2 3 6 7 010 011 110 111 - bit 1

    int bit0 = (pos.x > 0 ? 1 : 0);
    int bit1 = (pos.z > 0 ? 1 : 0) << 1;
    int bit2 = (pos.y < 0 ? 1 : 0) << 2;
    return bit2 | bit1 | bit0;
}

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
//    if(camPos.y > 0 && vertPos.y > cubeSize / 2.0) res.y = vertPos.y + camPos.y;
//    else res.y = vertPos.y;
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
//    float lifeDistance = cubeSize;
    int timeMod = int(ceil(lifeDistance / srf.x));
    vpos.y = xyzs.y - srf.x * (time % timeMod);
    vpos.x = xyzs.x + srf.y * cos(srf.z * time);
    vpos.z = xyzs.z + srf.y * sin(srf.z * time);

    vec3 cubeIndex = getCubeIndex(cameraPos);
    vec3 cubeCameraPos = cameraPos - cubeSize * cubeIndex;

    //adjust particle pos
    vpos = adjustPos(cubeCameraPos, vpos);

    //move all particles to current camera cube
    vpos += cubeSize * cubeIndex;

    //check visibility
    skipVertex = 1 - checkBit(octants, getOctant(vpos - cameraPos));

    gl_Position = vec4(vpos, 1.0);
    pass_xyzs = vec4(vpos, xyzs.w);

}

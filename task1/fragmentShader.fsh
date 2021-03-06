#version 330 core

in float zCoord;
out vec3 color;
uniform int drawOutline;
uniform int depthFillMethod;
uniform vec3 outlineColor;
uniform float near;
uniform float far;
uniform mat4 invP;

void main() {
   if(drawOutline == 1) {
       color = outlineColor;
   } else {
       float cval = 0.0;
       if(depthFillMethod == 0) {
           vec4 cPos = vec4(0.0, 0.0, 2.0 * gl_FragCoord.z - 1.0, 1.0) / gl_FragCoord.w;    //we need only z coord, so xy can be omitted
           vec4 ePos = invP * cPos;
           cval =  (-ePos.z - near) / (far - near);
       } else if(depthFillMethod == 1) {
           cval = (zCoord - near) / (far - near);
       }

       color = vec3(cval, cval, cval);
   }
}

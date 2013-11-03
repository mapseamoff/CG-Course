#version 330 core

in vec2 UV;
out vec3 color;
uniform int drawOutline;
uniform vec3 outlineColor;
uniform sampler2D texSampler;

void main() {
   if(drawOutline == 1) {
       color = outlineColor;
   } else {
       color = texture(texSampler, UV).rgb;
   }
}

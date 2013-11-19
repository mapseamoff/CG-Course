#version 330 core

in vec2 UV;
out vec3 color;
uniform int drawMipLevels;
uniform int drawOutline;
uniform vec3 outlineColor;
uniform sampler2D texSampler;

void main() {
    if(drawOutline == 1) {
        color = outlineColor;
    } else {
        if(drawMipLevels == 0) {
            color = texture(texSampler, UV).rgb;
        } else {
            ivec2 texSize = textureSize(texSampler, 0);

            float q = log2(texSize.x);
            float dudx = texSize.x * dFdx(UV.x);
            float dudy = texSize.y * dFdy(UV.x);
            float dvdx = texSize.x * dFdx(UV.y);
            float dvdy = texSize.y * dFdy(UV.y);
            float x = sqrt(dudx * dudx + dvdx * dvdx);
            float y = sqrt(dudy * dudy + dvdy * dvdy);
            float level = log2(max(x, y));
            if(level <= 0.5) {
                //color = texture(texSampler, UV).rgb;
                color = vec3(0, 0, 0);  //min level = source image
            } else if(level <= q + 0.5) {
                //as in opengl spec
                level = ceil(level + 0.5) - 1.0;
                //assume there is only 8 levels in texture :)
                color = vec3(0.125*level, 0.125*level, 0.125*level);
            } else {
                color = vec3(1, 1, 1); //max level
            }
        }
    }
}

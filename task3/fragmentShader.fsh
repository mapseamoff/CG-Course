#version 330 core

in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 spotDirection_cameraspace;
in vec3 vertexColor;

out vec3 color;

uniform int drawOutline;
uniform vec3 outlineColor;

uniform vec3 lightPosition_worldspace;

//-------------------------------------------------------------------------------------

uniform int shadingMethod;
uniform int fillMethod;
uniform int spotMethod;
uniform vec3 lightColor;
uniform float lightPower;
uniform float spotAngleCos;
uniform float spotExponent;
uniform vec3 diffuseColor;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float specularPower;

vec3 computeColor(vec3 pos, vec3 normal, vec3 lightPos, vec3 lightDir, vec3 viewDir, vec3 spotDir) {
    float distance = length(lightPos - pos);

    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir);
    float cosTheta = clamp(dot(N, L), 0, 1);

    vec3 V = normalize(viewDir);
    float cosAlpha = 0.0f;
    if(shadingMethod == 0) {
        vec3 R = reflect(-L, N);
        cosAlpha = clamp(dot(R, V), 0, 1);
    } else {
        vec3 H = normalize(V + L);
        cosAlpha = clamp(dot(H, N), 0, 1);
    }

    vec3 S = normalize(spotDir);
    float angleCos = clamp(dot(-S, L), 0, 1);
    float angleAttenuation = 0.0;
    if(angleCos >= spotAngleCos) {
        if(spotMethod == 0) angleAttenuation = pow(angleCos, spotExponent); //as in OpenGL spec
        else angleAttenuation = pow(clamp((angleCos - spotAngleCos) / (1.0 - spotAngleCos), 0, 1), spotExponent);
    }

    float distAttenuation = lightPower / (distance * distance);
    float attenuation = distAttenuation * angleAttenuation;

    return ambientColor + diffuseColor * lightColor * cosTheta * attenuation +
        specularColor * lightColor * pow(cosAlpha, specularPower) * attenuation;
}

//-------------------------------------------------------------------------------------

void main() {
    if(drawOutline == 1) {
        color = outlineColor;
    } else {
        if(fillMethod == 2) {
            color = computeColor(position_worldspace, normal_cameraspace,
                                 lightPosition_worldspace, lightDirection_cameraspace,
                                 eyeDirection_cameraspace, spotDirection_cameraspace);
        } else {
            color = vertexColor;
        }
    }
}

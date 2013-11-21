#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;

//out vec3 position_worldspace;
//out vec3 normal_cameraspace;
//out vec3 eyeDirection_cameraspace;
//out vec3 lightDirection_cameraspace;
//out vec3 vertexColor;

out vec3 pass_position_worldspace;
out vec3 pass_normal_cameraspace;
out vec3 pass_eyeDirection_cameraspace;
out vec3 pass_lightDirection_cameraspace;
out vec3 pass_spotDirection_cameraspace;
out vec3 pass_vertexColor;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPosition_worldspace;
uniform vec3 spotDirection_worldspace;

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
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
    vec3 lightPosition_cameraspace = (V * vec4(lightPosition_worldspace, 1)).xyz;

    pass_position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
    pass_eyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;
    pass_lightDirection_cameraspace = lightPosition_cameraspace - vertexPosition_cameraspace;
    pass_spotDirection_cameraspace = (V * vec4(spotDirection_worldspace, 0)).xyz;
    pass_normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;

    if(fillMethod == 1) {
        pass_vertexColor = computeColor(pass_position_worldspace, pass_normal_cameraspace,
                                   lightPosition_worldspace, pass_lightDirection_cameraspace,
                                   pass_eyeDirection_cameraspace, pass_spotDirection_cameraspace);
    } else {
        pass_vertexColor = vec3(0.5, 0.5, 0.5);
    }
}

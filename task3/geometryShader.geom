#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 pass_position_worldspace[];
in vec3 pass_normal_cameraspace[];
in vec3 pass_eyeDirection_cameraspace[];
in vec3 pass_lightDirection_cameraspace[];
in vec3 pass_spotDirection_cameraspace[];
in vec3 pass_vertexColor[];

out vec3 position_worldspace;
out vec3 normal_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 spotDirection_cameraspace;
out vec3 vertexColor;

uniform mat4 V;
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
    vec3 faceColor = vec3(0, 0, 0);
    if(fillMethod == 0) {
        vec3 faceNormal_cameraspace = vec3(0, 0, 0);
        vec3 faceCenter_worldspace = vec3(0, 0, 0);
        for(int i = 0; i < gl_in.length(); i++) {
            faceNormal_cameraspace += pass_normal_cameraspace[i];
            faceCenter_worldspace += pass_position_worldspace[i];
        }
        faceNormal_cameraspace = faceNormal_cameraspace / gl_in.length();
        faceCenter_worldspace = faceCenter_worldspace / gl_in.length();

        vec3 faceCenter_cameraspace = (V * vec4(faceCenter_worldspace, 1)).xyz;
        vec3 lightPosition_cameraspace = (V * vec4(lightPosition_worldspace, 1)).xyz;

        vec3 faceEyeDirection_cameraspace = -faceCenter_cameraspace;
        vec3 faceLightDirection_cameraspace = lightPosition_cameraspace - faceCenter_cameraspace;
        vec3 faceSpotDirection_cameraspace = pass_spotDirection_cameraspace[0];

        faceColor = computeColor(faceCenter_worldspace, faceNormal_cameraspace,
                                 lightPosition_worldspace, faceLightDirection_cameraspace,
                                 faceEyeDirection_cameraspace, faceSpotDirection_cameraspace);
    }


    for(int i = 0; i < gl_in.length(); i++) {
        position_worldspace = pass_position_worldspace[i];
        normal_cameraspace = pass_normal_cameraspace[i];
        eyeDirection_cameraspace = pass_eyeDirection_cameraspace[i];
        lightDirection_cameraspace = pass_lightDirection_cameraspace[i];
        spotDirection_cameraspace = pass_spotDirection_cameraspace[i];
        if(fillMethod == 0) {
            vertexColor = faceColor;
        } else {
            vertexColor = pass_vertexColor[i];
        }
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}

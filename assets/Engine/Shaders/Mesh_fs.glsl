#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2, binding = 4) uniform sampler2D u_DiffuseTexture;

layout (set = 2, binding = 3) uniform Material {
    vec4 Ks;
} g_Material;

#define MAX_LIGHTS 7

struct PointLight {
    vec3 Position;
    float RadiusRec;
    vec3 Light;
    float Padding;
};

layout (set = 0, binding = 0) uniform PerFrame {
    PointLight PointLights[MAX_LIGHTS];
    vec3 CameraPosition;
    uint NumLights;
    vec4 Padding;
} g_PerFrame;

layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec3 in_WorldPos;
layout (location = 3) in vec2 in_TXCoords;

layout (location = 0) out vec4 out_Color;

void main()
{
    vec3 normal = normalize(in_Normal);

    vec3 Kd = texture(u_DiffuseTexture, in_TXCoords).xyz;

    vec3 ambient = Kd * 0.08;
    vec3 pointLight = vec3(0.0);

    uint numLights = g_PerFrame.NumLights;
    for (uint lightIdx = 0; lightIdx < numLights; lightIdx += 1) {
        vec3 toLight        = g_PerFrame.PointLights[lightIdx].Position - in_WorldPos;
        float distToLight   = length(toLight);
        toLight /= distToLight;
        float cosAngle = min(dot(normal, toLight), 1.0);

        // Diffuse
        pointLight += cosAngle * g_PerFrame.PointLights[lightIdx].Light;

        // Specular
        vec3 toEye      = normalize(g_PerFrame.CameraPosition - in_WorldPos);
        vec3 halfwayVec = normalize(toLight + toEye);
        pointLight += g_PerFrame.PointLights[lightIdx].Light * pow(min(dot(halfwayVec, in_Normal), 1.0), g_Material.Ks.r) * g_Material.Ks.g;

        // Attenuation
        float attenuation = 1.0 - min(g_PerFrame.PointLights[lightIdx].RadiusRec * distToLight, 1.0);
        pointLight *= Kd * attenuation;
    }

    out_Color = vec4(min(ambient + pointLight, vec3(1.0)), 1.0);
}

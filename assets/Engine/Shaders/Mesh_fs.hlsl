Texture2D diffuseTX : register(t4);
SamplerState sampAni : register(s4);

cbuffer material : register(b3) {
    float4 Ks;
};

#define MAX_LIGHTS 7

struct PointLight {
    float3 Position;
    float3 Light;
    float RadiusRec;
};

cbuffer perFrame : register(b0) {
    PointLight pointLights[MAX_LIGHTS];
    float3 camPos;
    uint numLights;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 main(VS_OUT ps_in) : SV_TARGET {
    float3 normal = normalize(ps_in.normal);

    float3 Kd = diffuseTX.Sample(sampAni, ps_in.txCoords).xyz;

    float3 ambient = Kd * 0.08;

    float3 pointLight = (float3) 0;

    for (uint lightIdx = 0; lightIdx < numLights; lightIdx += 1) {
        float3 toLight = pointLights[lightIdx].Position - ps_in.worldPos;
        float distToLight = length(toLight);
        toLight /= distToLight;
        float cosAngle = dot(normal, toLight);
        if (cosAngle < 0.0) {
            continue;
        }

        // Diffuse
        pointLight += cosAngle * pointLights[lightIdx].Light;

        // Specular
        float3 toEye = normalize(camPos-ps_in.worldPos);
        float3 halfwayVec = normalize(toLight + toEye);
        pointLight += pointLights[lightIdx].Light * pow(saturate(dot(halfwayVec, normal)), Ks.r) * Ks.g;

        // Attenuation
        float attenuation = 1.0 - saturate(pointLights[lightIdx].RadiusRec * distToLight);
        pointLight *= Kd * attenuation;
    }

    return float4(saturate(ambient + pointLight), 1.0);
}

Texture2D diffuseTX : register(t0);
SamplerState sampAni;

cbuffer material : register(b0) {
    float4 Ks;
};

#define MAX_LIGHTS 7

struct PointLight {
    float3 lightPos;
    float lightRadiusReciprocal;
    float3 light;
    float padding;
};

cbuffer perFrame : register(b1) {
    PointLight pointLights[MAX_LIGHTS];
    float3 camPos;
    uint numLights;
    float4 padding;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 main(VS_OUT ps_in) : SV_TARGET {
    float3 normal = normalize(float3(ps_in.normal.xyz));

    float3 Kd = diffuseTX.Sample(sampAni, ps_in.txCoords).xyz;
    float3 finalColor = Kd * 0.08;

    float3 pointLight = (float3) 0;

    for (uint i = 0; i < numLights; i += 1) {
        float3 toLight = pointLights[i].lightPos - ps_in.worldPos;
        float distToLight = length(toLight);
        toLight /= distToLight;
        float cosAngle = saturate(dot(normal, toLight));

        // Diffuse
        pointLight += cosAngle * pointLights[i].light;

        // Specular
        float3 toEye = normalize(camPos-ps_in.worldPos);
        float3 halfwayVec = normalize(toLight + toEye);
        pointLight += pointLights[i].light.rgb * pow(saturate(dot(halfwayVec, ps_in.normal)), Ks.r) * Ks.g;

        // Attenuation
        float attenuation = 1.0 - saturate(pointLights[i].lightRadiusReciprocal * distToLight);
        pointLight *= Kd * attenuation;
    }

    return float4(saturate(finalColor + pointLight), 1.0);
}

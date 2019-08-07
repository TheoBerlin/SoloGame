Texture2D diffuseTX : register(t0);
SamplerState sampAni;

cbuffer material : register(b0) {
    float4 Ks;
};

#define MAX_LIGHTS 5

struct PointLight {
    float3 lightPos;
    float padA;
    float3 light;
    float padB;
    float lightRadiusReciprocal;
    float3 padC;
};

cbuffer perFrame : register(b1) {
    PointLight pointLights[MAX_LIGHTS];
    float3 camPos;
    float paddingD;
    uint numLights;
    uint paddingE[3];
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 PS_main(VS_OUT ps_in) : SV_TARGET {
    float3 normal = normalize(float3(ps_in.normal.xyz));

    float3 Kd = diffuseTX.Sample(sampAni, ps_in.txCoords).xyz;
    float3 finalColor = Kd * 0.2;

    for (uint i = 0; i < numLights; i += 1) {
        float3 toLight = pointLights[i].lightPos - ps_in.worldPos;
        float distToLight = length(toLight);
        toLight /= distToLight;
        float cosAngle = saturate(dot(normal, toLight));

        // Diffuse
        finalColor += cosAngle * pointLights[i].light;

        // Specular
        float3 toEye = normalize(camPos-ps_in.worldPos);
        float3 halfwayVec = normalize(toLight + toEye);
        finalColor += pointLights[i].light.rgb * pow(saturate(dot(halfwayVec, ps_in.normal)), Ks.r) * Ks.g;

        // Attenuation
        float attenuation = 1.0 - saturate(pointLights[i].lightRadiusReciprocal * distToLight);
        finalColor *= Kd * attenuation;
    }

    return float4(saturate(finalColor), 1.0);
}

Texture2D diffuseTX : register(t0);
SamplerState sampAni;


cbuffer material : register(b0) {
    float3 Ka;
    float3 Ks;
    float shininess;
};

#define MAX_LIGHTS 5

cbuffer PointLights : register(b1) {
    float3 lightPos[MAX_LIGHTS];
    float3 light[MAX_LIGHTS];
    float lightRadius[MAX_LIGHTS];
};

cbuffer NumLights : register(b2) {
    float numLights;
};

cbuffer camera : register(b3) {
    float3 camPos;
};

struct VS_OUT {
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 PS_main(VS_OUT ps_in) : SV_TARGET {
    float3 normal = normalize(float3(ps_in.normal.xyz));

    float3 ambient = (float3) 0;

    float3 Kd = diffuseTX.Sample(sampAni, ps_in.txCoords).xyz;
    float3 diffuse = (float3) 0;

    float3 specular = (float3) 0;

    for (int i = 0; i < numLights; i += 1) {
        float distance = length(lightPos[i] - ps_in.worldPos);
        float3 lightDir = (lightPos[i] - ps_in.worldPos)/distance;
        float cosAngle = dot(normal, lightDir);
        float attenuation = 1.0/distance;

        ambient += Ka*light[i]*attenuation;
        diffuse += max(cosAngle, 0.0) * Kd * light[i] * attenuation;
        specular += pow(max(0.0, dot(reflect(lightDir, normal), normalize(camPos-ps_in.worldPos))), shininess)*light[i];
    }

    diffuse = saturate(diffuse);
    specular = saturate(specular);

    return float4(ambient + diffuse + specular, 1.0);
}

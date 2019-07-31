Texture2D diffuseTX : register(t0);
SamplerState sampAni;

cbuffer material : register(b0) {
    float3 Ka;
    float3 Ks;
    float shininess;
};

#define MAX_LIGHTS 5

struct PointLight {
    float3 lightPos;
    float3 light;
    float lightRadius;
};

cbuffer perFrame : register(b1) {
    PointLight pointLights[MAX_LIGHTS];
    float3 camPos;
    uint numLights;
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

    for (uint i = 0; i < numLights; i += 1) {
        float distance = length(pointLights[i].lightPos - ps_in.worldPos);
        float3 lightDir = (pointLights[i].lightPos - ps_in.worldPos)/distance;
        float cosAngle = dot(normal, lightDir);
        float attenuation = 1.0/distance;

        ambient += Ka*pointLights[i].light*attenuation;
        diffuse += max(cosAngle, 0.0) * Kd * pointLights[i].light * attenuation;
        specular += pow(max(0.0, dot(reflect(lightDir, normal), normalize(camPos-ps_in.worldPos))), shininess)*pointLights[i].light;
    }

    diffuse = saturate(diffuse);
    specular = saturate(specular);

    return float4(ambient + diffuse + specular, 1.0);
}

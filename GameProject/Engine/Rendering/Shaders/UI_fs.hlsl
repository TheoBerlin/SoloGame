Texture2D uiTexture : register(t4);
SamplerState sampAni : register(s1);

cbuffer perObject : register(b2)
{
    float2 position, size;
    float4 highlight;
    float highlightFactor;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 main(VS_OUT ps_in) : SV_TARGET {
    float4 txColor = uiTexture.Sample(sampAni, ps_in.txCoords);
    return saturate(txColor + highlightFactor * txColor * highlight);
}

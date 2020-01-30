Texture2D uiTexture : register(t0);
SamplerState sampAni;

cbuffer perObject : register(b0)
{
    float2 position, size;
    float4 highlight;
    float highlightFactor;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 PS_main(VS_OUT ps_in) : SV_TARGET {
    float4 txColor = uiTexture.Sample(sampAni, ps_in.txCoords);
    return saturate(txColor + highlightFactor * txColor * highlight);
    //return color * uiTexture.Sample(sampAni, ps_in.txCoords);
}

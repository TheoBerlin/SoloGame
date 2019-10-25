//Texture2D uiTexture : register(t0);
//SamplerState sampAni;

cbuffer perObject : register(b0)
{
    float2 position, size;
    float4 color;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 txCoords : TEXCOORD0;
};

float4 PS_main(VS_OUT ps_in) : SV_TARGET {
    return color;// * uiTexture.Sample(sampAni, ps_in.txCoords);
}

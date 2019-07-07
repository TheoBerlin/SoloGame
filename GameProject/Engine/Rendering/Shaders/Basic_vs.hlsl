cbuffer perObject : register(b0)
{
    float4x4 wvp;
    float4x4 world;
};

struct VS_IN {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 txCoords : TEXCOORD0;
};

struct VS_OUT {
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
    float2 txCoords : TEXCOORD0;
};

VS_OUT VS_main(VS_IN v_in) {
    VS_OUT v_out = (VS_OUT) 0;

    v_out.pos = float4(v_in.pos, 1.0);
    v_out.normal = mul(float4(v_in.normal, 0.0), world).xyz;
    v_out.worldPos = mul(v_out.pos, wvp).xyz;
    v_out.txCoords = v_in.txCoords;

    return v_out;
}

cbuffer perObject : register(b0)
{
    float2 position, size;
    float4 color;
};

struct VS_IN {
    float2 pos : POSITION;
    float2 txCoords : TEXCOORD0;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 txCoords : TEXCOORD0;
};

VS_OUT VS_main(VS_IN v_in) {
    VS_OUT v_out = (VS_OUT) 0;

    // Resize the quad, timed by two because the size factors are in [0,1] but the vertex position
    // are described in [-1,1], a twice as large interval:
    // v_in.pos * size * 2
    // Translate the quad by translating whilst converting positions in [0,1] to [-1,1]:
    // + position * 2.0 - 1:
    v_out.pos = float4(v_in.pos * size * 2 + position * 2.0 - 1, 0.0, 1.0);
    v_out.txCoords = v_in.txCoords;

    return v_out;
}


cbuffer Color : register(b3) {
    float3 color;
};

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    return float4(color, 1.0f);
}

Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input {
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv  : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    return float4(0, 1, 0, 1);
    // return srv.Sample(samplerState, i.uv);
}

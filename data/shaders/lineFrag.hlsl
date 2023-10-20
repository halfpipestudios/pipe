Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input {
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv  : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET {

    float3 color = float3(0, 1, 0);

    float3 dir = normalize(float3(1, 1, 1));
    float ambient = max(dot(dir, i.nor), 0.1f);
    color = color * ambient;
    return float4(color, 1.0f);
    // return srv.Sample(samplerState, i.uv);
}

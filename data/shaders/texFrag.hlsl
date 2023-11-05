Texture2DArray srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    float3 color = srv.Sample(samplerState, float3(i.uv, i.tex)).rgb;
    float3 dir = normalize(float3(-0.2f, 0.5f, 1));
    float ambient = max(dot(dir, i.nor), 0.3f);
    color = color * ambient;
    return float4(color, 1.0f);
}

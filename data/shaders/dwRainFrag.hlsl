Texture2DArray srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input {
	float4 PosH  : SV_Position;
	float2 Tex   : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    return srv.Sample(samplerState, float3(i.Tex, 0));
}

Texture2DArray srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
	float4 pos   : SV_POSITION;
	float2 uvs   : TEXCOORD0;
	float3 color : COLOR;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    return srv.Sample(samplerState, float3(i.uvs, 0)) * float4(i.color, 1.0);
}


cbuffer CBuffer : register(b5)
{
	unsigned int resX;
	unsigned int resY;
	unsigned int pad0;
	unsigned int pad1;
}

struct VS_Input
{
	float2 pos : POSITION;
	float2 uvs : TEXCOORD0;
	float3 color : COLOR;
};

struct PS_Input
{
	float4 pos : SV_POSITION;
	float2 uvs : TEXCOORD0;
	float3 color : COLOR;
};

PS_Input vs_main(VS_Input vertex) {
    PS_Input vsOut = (PS_Input)0;
	
    float2 screen = float2(resX, resY);
    
	vsOut.pos.x = (vertex.pos.x / screen.x) *  2 - 1;
    vsOut.pos.y = (vertex.pos.y / screen.y) * -2 + 1;
    
    vsOut.pos.z = 0;
    vsOut.pos.w = 1;

    vsOut.uvs   = vertex.uvs;
    vsOut.color = vertex.color;

    return vsOut;
}


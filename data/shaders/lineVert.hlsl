struct VS_Input
{
    float3 pos : POSITION;
    float4 col : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_Input vs_main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input)0;
    vsOut.pos = float4(vertex.pos, 1.0f);
    vsOut.col = vertex.col;
    return vsOut;
}

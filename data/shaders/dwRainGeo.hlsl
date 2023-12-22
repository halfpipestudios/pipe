
cbuffer CBMatrices : register(b0) {
    matrix proj;
    matrix view;
    matrix world;
}

cbuffer CBParticle : register(b2) {

    float3 eyePosW;
    float gameTime;

    float3 emitPosW;
    float timeStep;
    
    float3 emitDirW;
}

static float2 gQuadTexC[4] = {
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f)
};

struct GeoIn
{
	float3 PosW  : POSITION;
	unsigned int   Type  : TEXCOORD0;
};

struct GeoOut
{
	float4 PosH  : SV_Position;
	float2 Tex   : TEXCOORD0;
};

#define PT_EMITTER 0
#define PT_FLARE 1

static float3 gAccelW = { -1.0f, -9.8f, 0.0f };

[maxvertexcount(2)]
void GS(point GeoIn gin[1],
        inout LineStream<GeoOut> lineStream) {
    // do not draw emitter particles
    if(gin[0].Type != PT_EMITTER) {
        // Slant line in acceleration direction
        float3 p0 = gin[0].PosW;
        float3 p1 = gin[0].PosW + 0.07f*gAccelW;

        GeoOut v0;
        float4 wPos = float4(p0, 1.0f);
        wPos = mul(wPos, view);
        v0.PosH = mul(wPos, proj);
        v0.Tex = float2(0, 0);
        lineStream.Append(v0);

        GeoOut v1;
        wPos = float4(p1, 1.0f);
        wPos = mul(wPos, view);
        v1.PosH = mul(wPos, proj);
        v1.Tex = float2(1, 1);
        lineStream.Append(v1);
    }
}

cbuffer CBParticle : register(b2) {

    float3 eyePosW;
    float gameTime;

    float3 emitPosW;
    float timeStep;
    
    float3 emitDirW;
}

static float3 gAccelW = {0.0f, 0.0f, 0.0f };

struct VS_Input {
    float3 pos        : POSITION;
    float3 vel        : TEXCOORD0;
    float2 sizeW      : TEXCOORD1;
    float age         : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

struct PS_Input {
	float3 PosW  : POSITION;
	float2 SizeW : TEXCOORD0;
	float4 Color : TEXCOORD1;
	unsigned int   Type  : TEXCOORD2;
};

PS_Input vs_main(VS_Input vin) {
    PS_Input vout;
    
    float t  = vin.age;
    
    vout.PosW = 0.5f*t*t*gAccelW + t*vin.vel + vin.pos;
    // fade color with time
    float opacity = 1.0f - smoothstep(0.0f, 0.25f, t/1.0f);
    vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

    vout.SizeW = vin.sizeW;
    vout.Type = vin.type;

    return vout;
}

cbuffer CBParticle : register(b2) {

    float3 eyePosW;
    float gameTime;

    float3 emitPosW;
    float timeStep;
    
    float3 emitDirW;
    float pad0;

    float3 targetPosW;
    float pad1;
}

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
    //float3 target = float3(0, 2.75f, 0);
    float3 target = targetPosW;
    
    float3 dirA = vin.vel;
    float3 dirB = target - vin.pos;
    
    float mix  = smoothstep(0.0f, 1.0f, t);
    float3 dir = lerp(dirA, dirB, mix);
    
    // constant accelertion equation
    vout.PosW = vin.pos + dir * t;
    vout.Color = float4(1.0f, 1.0f, 1.0f, 1);

    vout.SizeW = vin.sizeW;
    vout.Type = vin.type;

    return vout;
}

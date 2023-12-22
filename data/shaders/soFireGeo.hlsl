
Texture1D gRandomTex;
SamplerState samLinear;

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

struct Particle {
    float3 pos        : POSITION;
    float3 vel        : TEXCOORD0;
    float2 sizeW      : TEXCOORD1;
    float age         : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

float3 RandomUnitVec3(float offfset) {
    float u = (gameTime + offfset);
    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
    return normalize(v);
}

#define PT_EMITTER 0
#define PT_FLARE 1

[maxvertexcount(2)]
void GS(point Particle gin[1],
        inout PointStream<Particle> ptStream) {

    gin[0].age += timeStep;
    if(gin[0].type == PT_EMITTER) {
        // time to emit a new particle?
        if(gin[0].age > 0.005f) {
            float3 vRandom = RandomUnitVec3(0.0f);
            vRandom.x *= 0.5f;
            vRandom.z *= 0.5f;

            Particle p;
            p.pos = emitPosW;
            p.vel = 4.0f*vRandom;
            p.sizeW = float2(3.0f, 3.0f);
            p.age = 0.0f;
            p.type = PT_FLARE;

            ptStream.Append(p);

            // reset the time to emit
            gin[0].age = 0.0f;
        }
        // always keep emitters
        ptStream.Append(gin[0]);
    } else {
        // Specify condition to keep particle; this may vary from system to system.
        if(gin[0].age <= 1.0f)
            ptStream.Append(gin[0]);
    }
}
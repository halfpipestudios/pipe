cbuffer Matrices : register(b0) {
    matrix proj;
    matrix view;
    matrix world;
};

static const int MAX_BONES = 100;
static const int MAX_BONE_INFLUENCE = 4;

cbuffer Animation : register(b1) {
    matrix boneMatrix[MAX_BONES];
};

struct VS_Input {
    float3 pos     : POSITION;
    float3 nor     : NORMAL;
    float2 uv      : TEXCOORD0;
    int4 boneIds   : TEXCOORD1;
    float4 weigths : TEXCOORD2;
};

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
    float3 fragPos   : TEXCOORD2;
};

PS_Input vs_main(VS_Input input) {
    PS_Input o = (PS_Input)0;
    
    float4 totalPosition = float4(0, 0, 0, 0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if(input.boneIds[i] == -1) {
            continue;
        }
        
        if(input.boneIds[i] >= MAX_BONES) {
            totalPosition = float4(input.pos, 1.0f);
            break;
        }

        float4 localPosition = mul(float4(input.pos, 1.0f), boneMatrix[input.boneIds[i]]);
        totalPosition += localPosition * input.weigths[i];
    }
    
    float4 wPos =  mul(totalPosition, world);
    float3 fragPos = float3(wPos.xyz);
    wPos = mul(wPos, view);
    wPos = mul(wPos, proj);

    float3 wNor = mul(input.nor, (float3x3)world);
    wNor = normalize(wNor);

    o.pos = wPos;
    o.nor = wNor;
    o.uv = input.uv;
    o.tex = 0; 
    o.fragPos = fragPos;
    
    return o;
}

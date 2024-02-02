Texture2DArray srv : register(t0);
SamplerState samplerState : register(s0);

struct Light {

    float3 pos;
    int type;

    float3 dir;
    float cutOff;

    float3 ambient;
    float constant;

    float3 diffuse;
    float linear_;

    float3 specular;
    float quadratic;

    float outerCutOff;
    float pad0;
    float pad1;
    float pad2;

};

#define MAX_LIGHTS_COUNT 64
cbuffer CLightBuffer : register(b6) {
    int count;
    float3 viewPos;
    Light lights[MAX_LIGHTS_COUNT];
    float farPlane;
    float pad3;
    float pad4;
    float pad5;
};

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
    float3 fragPos   : TEXCOORD2;
};

struct PS_Out {
    float4 color : SV_Target;
    float  depth : SV_Depth;
};


PS_Out fs_main(PS_Input i) {

    PS_Out output = (PS_Out)0;
    float3 color = srv.Sample(samplerState, float3(i.uv, 0)).rgb;
    float lightDistance = length(i.fragPos - viewPos);
    lightDistance = lightDistance / farPlane;

    output.color = float4(color, 1.0f);
    output.depth = lightDistance;

    return output;
}

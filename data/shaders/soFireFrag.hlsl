struct PS_Input {
    float3 pos        : POSITION;
    float3 vel        : TEXCOORD0;
    float2 sizeW      : TEXCOORD1;
    float age         : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

float4 fs_main(PS_Input i) : SV_TARGET {
    return float4(1, 1, 1, 1);
}

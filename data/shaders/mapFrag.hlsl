#define MAX_LIGHTS_COUNT 64
#define LIGHT_TYPE_UNDEFINE 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_POINT 2
#define LIGHT_TYPE_SPOT 3

Texture2DArray srv : register(t0);
TextureCube depthMap[MAX_LIGHTS_COUNT] : register(t1);

SamplerState samplerState : register(s0);
SamplerState depthMapSampler : register(s1);

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
    int shadowMapIndex;
    float pad1;
    float pad2;

};

cbuffer CLightBuffer : register(b6) {
    int count;
    float3 viewPos;
    Light lights[MAX_LIGHTS_COUNT];
    float farPlane;
    float pad3;
    float pad4;
    float pad5;
};

cbuffer CMaterial : register(b7) {
    float3 ambient;
    float pad0;
    float3 diffuse;
    float pad1;
    float3 specular;
    float shininess;
};

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
    float3 fragPos   : TEXCOORD2;
};

float ShadowCalculation(Light light, float3 fragPos, float3 lightDir, float3 normal) {
    float3 fragToLight = fragPos - light.pos;

    float closestDepth = 0.0f;

    switch(light.shadowMapIndex)
    {
        case 0: closestDepth = depthMap[0].Sample(depthMapSampler, fragToLight).r;  break;
        case 1: closestDepth = depthMap[1].Sample(depthMapSampler, fragToLight).r;  break;
        case 2: closestDepth = depthMap[2].Sample(depthMapSampler, fragToLight).r;  break;
        case 3: closestDepth = depthMap[3].Sample(depthMapSampler, fragToLight).r;  break;
        case 4: closestDepth = depthMap[4].Sample(depthMapSampler, fragToLight).r;  break;
        case 5: closestDepth = depthMap[5].Sample(depthMapSampler, fragToLight).r;  break;
        case 6: closestDepth = depthMap[6].Sample(depthMapSampler, fragToLight).r;  break;
        case 7: closestDepth = depthMap[7].Sample(depthMapSampler, fragToLight).r;  break;
        case 8: closestDepth = depthMap[8].Sample(depthMapSampler, fragToLight).r;  break;
        case 9: closestDepth = depthMap[9].Sample(depthMapSampler, fragToLight).r;  break;

        case 10: closestDepth = depthMap[10].Sample(depthMapSampler, fragToLight).r;  break;
        case 11: closestDepth = depthMap[11].Sample(depthMapSampler, fragToLight).r;  break;
        case 12: closestDepth = depthMap[12].Sample(depthMapSampler, fragToLight).r;  break;
        case 13: closestDepth = depthMap[13].Sample(depthMapSampler, fragToLight).r;  break;
        case 14: closestDepth = depthMap[14].Sample(depthMapSampler, fragToLight).r;  break;
        case 15: closestDepth = depthMap[15].Sample(depthMapSampler, fragToLight).r;  break;
        case 16: closestDepth = depthMap[16].Sample(depthMapSampler, fragToLight).r;  break;
        case 17: closestDepth = depthMap[17].Sample(depthMapSampler, fragToLight).r;  break;
        case 18: closestDepth = depthMap[18].Sample(depthMapSampler, fragToLight).r;  break;
        case 19: closestDepth = depthMap[19].Sample(depthMapSampler, fragToLight).r;  break;

        case 20: closestDepth = depthMap[20].Sample(depthMapSampler, fragToLight).r;  break;
        case 21: closestDepth = depthMap[21].Sample(depthMapSampler, fragToLight).r;  break;
        case 22: closestDepth = depthMap[22].Sample(depthMapSampler, fragToLight).r;  break;
        case 23: closestDepth = depthMap[23].Sample(depthMapSampler, fragToLight).r;  break;
        case 24: closestDepth = depthMap[24].Sample(depthMapSampler, fragToLight).r;  break;
        case 25: closestDepth = depthMap[25].Sample(depthMapSampler, fragToLight).r;  break;
        case 26: closestDepth = depthMap[26].Sample(depthMapSampler, fragToLight).r;  break;
        case 27: closestDepth = depthMap[27].Sample(depthMapSampler, fragToLight).r;  break;
        case 28: closestDepth = depthMap[28].Sample(depthMapSampler, fragToLight).r;  break;
        case 29: closestDepth = depthMap[29].Sample(depthMapSampler, fragToLight).r;  break;

        case 30: closestDepth = depthMap[30].Sample(depthMapSampler, fragToLight).r;  break;
        case 31: closestDepth = depthMap[31].Sample(depthMapSampler, fragToLight).r;  break;
        case 32: closestDepth = depthMap[32].Sample(depthMapSampler, fragToLight).r;  break;
        case 33: closestDepth = depthMap[33].Sample(depthMapSampler, fragToLight).r;  break;
        case 34: closestDepth = depthMap[34].Sample(depthMapSampler, fragToLight).r;  break;
        case 35: closestDepth = depthMap[35].Sample(depthMapSampler, fragToLight).r;  break;
        case 36: closestDepth = depthMap[36].Sample(depthMapSampler, fragToLight).r;  break;
        case 37: closestDepth = depthMap[37].Sample(depthMapSampler, fragToLight).r;  break;
        case 38: closestDepth = depthMap[38].Sample(depthMapSampler, fragToLight).r;  break;
        case 39: closestDepth = depthMap[39].Sample(depthMapSampler, fragToLight).r;  break;

        case 40: closestDepth = depthMap[40].Sample(depthMapSampler, fragToLight).r;  break;
        case 41: closestDepth = depthMap[41].Sample(depthMapSampler, fragToLight).r;  break;
        case 42: closestDepth = depthMap[42].Sample(depthMapSampler, fragToLight).r;  break;
        case 43: closestDepth = depthMap[43].Sample(depthMapSampler, fragToLight).r;  break;
        case 44: closestDepth = depthMap[44].Sample(depthMapSampler, fragToLight).r;  break;
        case 45: closestDepth = depthMap[45].Sample(depthMapSampler, fragToLight).r;  break;
        case 46: closestDepth = depthMap[46].Sample(depthMapSampler, fragToLight).r;  break;
        case 47: closestDepth = depthMap[47].Sample(depthMapSampler, fragToLight).r;  break;
        case 48: closestDepth = depthMap[48].Sample(depthMapSampler, fragToLight).r;  break;
        case 49: closestDepth = depthMap[49].Sample(depthMapSampler, fragToLight).r;  break;

        case 50: closestDepth = depthMap[50].Sample(depthMapSampler, fragToLight).r;  break;
        case 51: closestDepth = depthMap[51].Sample(depthMapSampler, fragToLight).r;  break;
        case 52: closestDepth = depthMap[52].Sample(depthMapSampler, fragToLight).r;  break;
        case 53: closestDepth = depthMap[53].Sample(depthMapSampler, fragToLight).r;  break;
        case 54: closestDepth = depthMap[54].Sample(depthMapSampler, fragToLight).r;  break;
        case 55: closestDepth = depthMap[55].Sample(depthMapSampler, fragToLight).r;  break;
        case 56: closestDepth = depthMap[56].Sample(depthMapSampler, fragToLight).r;  break;
        case 57: closestDepth = depthMap[57].Sample(depthMapSampler, fragToLight).r;  break;
        case 58: closestDepth = depthMap[58].Sample(depthMapSampler, fragToLight).r;  break;
        case 59: closestDepth = depthMap[59].Sample(depthMapSampler, fragToLight).r;  break;

        case 60: closestDepth = depthMap[60].Sample(depthMapSampler, fragToLight).r;  break;
        case 61: closestDepth = depthMap[61].Sample(depthMapSampler, fragToLight).r;  break;
        case 62: closestDepth = depthMap[62].Sample(depthMapSampler, fragToLight).r;  break;
        case 63: closestDepth = depthMap[63].Sample(depthMapSampler, fragToLight).r;  break;
    }

    closestDepth *= farPlane;
    float currentDepth = length(fragToLight);
    float shadow = currentDepth - 0.005f > closestDepth ? 1.0f : 0.0f;
    return shadow;
}

float3 CalcDirLight(float3 color, Light light, float3 normal, float3 viewDir) {
    float3 lightDir = normalize(-light.dir);
    float diff = max(dot(normal, lightDir), 0.1f);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);

    float3 ambient_  = light.ambient  * (color * ambient);
    float3 diffuse_  = light.diffuse  * diff * (color * diffuse);
    float3 specular_ = light.specular * spec * (color * specular); 
    return ambient_ + diffuse_ + specular_;
}


float3 CalcPointLight(float3 color, Light light, float3 normal, float3 viewDir, float3 fragPos) {

    float3 lightDir = normalize(light.pos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0f);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);

    float dist = length(light.pos - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear_ * dist + light.quadratic * (dist * dist));

    float3 ambient_  = light.ambient  * (color * ambient);
    float3 diffuse_  = light.diffuse  * diff * (color * diffuse);
    float3 specular_ = light.specular * spec * (color * specular); 

    ambient_ *= attenuation;
    diffuse_ *= attenuation;
    specular_ *= attenuation;

    // return ambient_ + diffuse_ + specular_;
    float shadow = ShadowCalculation(light, fragPos, lightDir, normal);
    return (ambient_ + (1.0f - shadow) * (diffuse_ + specular_));
    //return (1.0f - shadow) * (ambient_ + diffuse_ + specular_);
}

float4 fs_main(PS_Input i) : SV_TARGET {
    //float3 frameBuffer = (float3(1, 1, 1) - frameBufferSrv.Sample(samplerState, float3(i.uv, 0)).rrr) * 4.0f;
    //return float4(frameBuffer, 1);


    float3 color = srv.Sample(samplerState, float3(i.uv, i.tex)).rgb;
    float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);

    float3 result = float3(0.0f, 0.0f, 0.0f);

    for(int index = 0; index < count; index++) {
        Light light = lights[index];
        switch(light.type) {
            case LIGHT_TYPE_DIRECTIONAL: {
                result += CalcDirLight(color, light, normal, viewDir);
            } break;
            case LIGHT_TYPE_POINT: {
                result += CalcPointLight(color, light, normal, viewDir, i.fragPos);
            } break;
            case LIGHT_TYPE_SPOT: {

            } break;
        }
    }

    return float4(result, 1.0f);
}

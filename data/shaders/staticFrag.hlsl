#define MAX_LIGHTS_COUNT 64
#define LIGHT_TYPE_UNDEFINE 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_POINT 2
#define LIGHT_TYPE_SPOT 3

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

    return ambient_ + diffuse_ + specular_;
}

float4 fs_main(PS_Input i) : SV_TARGET {
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

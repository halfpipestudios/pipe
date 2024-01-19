#ifndef _PARTICLE_CMP_H_
#define _PARTICLE_CMP_H_

#include "graphics.h"

#define MAX_PARTICLE_SHADER_NAME 32

struct ParticleCMP : CMP<ParticleCMP> {
    
    ParticleSystem particleSys { nullptr };

    char soVName[MAX_PARTICLE_SHADER_NAME];
    char soFName[MAX_PARTICLE_SHADER_NAME];
    char soGName[MAX_PARTICLE_SHADER_NAME];
    char dwVName[MAX_PARTICLE_SHADER_NAME];
    char dwFName[MAX_PARTICLE_SHADER_NAME];
    char dwGName[MAX_PARTICLE_SHADER_NAME];

    void Initialize(u32 maxParticles,
                    char *soVName_, char *soFName_, char *soGName_,
                    char *dwVName_, char *dwFName_, char *dwGName_,
                    Handle texture) {

        strcpy(soVName, soVName_);
        strcpy(soFName, soFName_);
        strcpy(soGName, soGName_);
        strcpy(dwVName, dwVName_);
        strcpy(dwFName, dwFName_);
        strcpy(dwGName, dwGName_);

        Handle soVHandle =   VShaderManager::Get()->GetAsset(soVName_);
        Handle soFHandle =   FShaderManager::Get()->GetAsset(soFName_);
        Handle soGHandle = GSOShaderManager::Get()->GetAsset(soGName_);
        Handle dwVHandle =   VShaderManager::Get()->GetAsset(dwVName_);
        Handle dwFHandle =   FShaderManager::Get()->GetAsset(dwFName_);
        Handle dwGHandle =   GShaderManager::Get()->GetAsset(dwGName_);

        VShader soV = *VShaderManager::Get()->Dereference(soVHandle);
        FShader soF = *FShaderManager::Get()->Dereference(soFHandle);
        GeometryShader soG = *GSOShaderManager::Get()->Dereference(soGHandle);
        VShader dwV = *VShaderManager::Get()->Dereference(dwVHandle);
        FShader dwF = *FShaderManager::Get()->Dereference(dwFHandle);
        GeometryShader dwG = *GShaderManager::Get()->Dereference(dwGHandle);

        particleSys = GraphicsManager::Get()->CreateParticleSystem(maxParticles,
                soV, soF, soG, dwV, dwF, dwG, texture);

    }

    void Terminate() override {
        if(particleSys) GraphicsManager::Get()->DestroyParticleSystem(particleSys);
        printf("ParticleCMP terminate\n"); 
    } 

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "particle");
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "particle");
        ReadEndObject(t);
    };

};

#endif // _PARTICLE_CMP_H_

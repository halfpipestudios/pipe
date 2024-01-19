#ifndef _PARTICLE_CMP_H_
#define _PARTICLE_CMP_H_

#include "graphics.h"

#define MAX_PARTICLE_SHADER_NAME 32
#define MAX_PARTICLE_TEXTURE_NAME 32

struct ParticleCMP : CMP<ParticleCMP> {
    
    ParticleSystem particleSys { nullptr };
    
    u32 maxParticles;
    
    char soVName[MAX_PARTICLE_SHADER_NAME];
    char soFName[MAX_PARTICLE_SHADER_NAME];
    char soGName[MAX_PARTICLE_SHADER_NAME];
    char dwVName[MAX_PARTICLE_SHADER_NAME];
    char dwFName[MAX_PARTICLE_SHADER_NAME];
    char dwGName[MAX_PARTICLE_SHADER_NAME];
    
    char textureName[MAX_PARTICLE_TEXTURE_NAME];

    void Initialize(u32 maxParticles_,
                    char *soVName_, char *soFName_, char *soGName_,
                    char *dwVName_, char *dwFName_, char *dwGName_,
                    char *textureName_) {
        
        maxParticles = maxParticles_;

        strcpy(textureName, textureName_);
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
        
        Handle texture = TextureManager::Get()->GetAsset(textureName);
        particleSys = GraphicsManager::Get()->CreateParticleSystem(maxParticles,
                soV, soF, soG, dwV, dwF, dwG, texture);

    }

    void Terminate() override {
        if(particleSys) GraphicsManager::Get()->DestroyParticleSystem(particleSys);
        printf("ParticleCMP terminate\n"); 
    } 

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "particle");
        Write(s, "max_particles", maxParticles);
        Write(s, "soVName", soVName);
        Write(s, "soFName", soFName);
        Write(s, "soGName", soGName);
        Write(s, "dwVName", dwVName);
        Write(s, "dwFName", dwFName);
        Write(s, "dwGName", dwGName);
        Write(s, "texture", textureName);
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        
        u32 maxParticles_;
        char soVName_[MAX_PARTICLE_SHADER_NAME];
        char soFName_[MAX_PARTICLE_SHADER_NAME];
        char soGName_[MAX_PARTICLE_SHADER_NAME];
        char dwVName_[MAX_PARTICLE_SHADER_NAME];
        char dwFName_[MAX_PARTICLE_SHADER_NAME];
        char dwGName_[MAX_PARTICLE_SHADER_NAME];
        char textureName_[MAX_PARTICLE_TEXTURE_NAME];

        ReadBeginObject(t, "particle");
        Read(t, "max_particles", &maxParticles_);
        Read(t, "soVName", soVName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "soFName", soFName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "soGName", soGName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "dwVName", dwVName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "dwFName", dwFName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "dwGName", dwGName_, MAX_PARTICLE_SHADER_NAME);
        Read(t, "texture", textureName_, MAX_PARTICLE_TEXTURE_NAME);
        ReadEndObject(t);
        
        Initialize(maxParticles_, soVName_, soFName_, soGName_, dwVName_, dwFName_, dwGName_, textureName_);
    };

};

#endif // _PARTICLE_CMP_H_

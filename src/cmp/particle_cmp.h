#ifndef _PARTICLE_CMP_H_
#define _PARTICLE_CMP_H_

#include "graphics.h"

struct ParticleCMP : CMP<ParticleCMP> {
    
    ParticleSystem particleSys { nullptr };
    void Initialize(u32 maxParticles,
                    Shader soShader, GeometryShader soGeoShader,
                    Shader dwShader, GeometryShader dwGeoShader,
                    Handle texture) {

        particleSys = GraphicsManager::Get()->CreateParticleSystem(maxParticles,
                soShader, soGeoShader, dwShader, dwGeoShader, texture);

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

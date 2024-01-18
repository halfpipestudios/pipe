#ifndef _FIRE_SPELL_CMP_
#define _FIRE_SPELL_CMP_

#include "graphics.h"

struct Fireball {
    bool active { false };
    f32 life  { 0.0f };
    f32 renderTimer { 0.0f };
    f32 spinDir { 1.0f };
    Vec3 pos;
    Vec3 vel;
    ParticleSystem particleSys { nullptr };
};

struct FireSpellCMP : CMP<FireSpellCMP> {
    
    f32 damage { 100 };
    Fireball fireballs[20] {};
    i32 nextToShoot { 0 };
    f32 cooldown  { 0.25f };
    f32 lifeTime { 2.0f };
    f32 timer     { 0.0f };
    f32 speed    { 10.0f };

    void Initialize(Shader soShader, GeometryShader soGeoShader,
                    Shader dwShader, GeometryShader dwGeoShader,
                    Handle texture) {

        for(i32 i = 0; i < ARRAY_LENGTH(fireballs); i++) {
            Fireball *fireball = fireballs + i;
            fireball->renderTimer = RandF32(0, 2.0f*(f32)PI);
            if((rand() % 2) == 0) {
                fireball->spinDir = -1.0f;
            }
            else
            {
                fireball->spinDir = 1.0f;
            }

            fireball->particleSys = 
                GraphicsManager::Get()->CreateParticleSystem(1000, soShader, soGeoShader,
                                                             dwShader, dwGeoShader, texture);
        }
    }

    void Terminate() override {
        for(i32 i = 0; i < ARRAY_LENGTH(fireballs); i++) {
            Fireball *fireball = fireballs + i;
            if(fireball->particleSys) {
                GraphicsManager::Get()->DestroyParticleSystem(fireball->particleSys);
            }
        }
        printf("Fire Spell Component Terminate\n");
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "fire_spell");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "fire_spell");
        ReadEndObject(t);
    };
};

#endif

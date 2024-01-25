#ifndef _PLAYER_CMP_H_
#define _PLAYER_CMP_H_

#include "graphics_manager.h"
#include "mgr/shader_manager.h"
#include "mgr/texture_manager.h"
#include "sound.h"

struct PlayerCMP : CMP<PlayerCMP> {
    
    Sound jumpSound;
    ParticleSystem particleSys { nullptr };

    void Initialize() {
        jumpSound.Initialize("jump.wav");

        VShader soVShader = *VShaderManager::Get()->Dereference(VShaderManager::Get()->GetAsset("soSpellVert.hlsl"));
        FShader soFShader = *FShaderManager::Get()->Dereference(FShaderManager::Get()->GetAsset("soSpellFrag.hlsl"));
        GeometryShader soGShader = *GSOShaderManager::Get()->Dereference(GSOShaderManager::Get()->GetAsset("soSpellGeo.hlsl"));

        VShader dwVShader = *VShaderManager::Get()->Dereference(VShaderManager::Get()->GetAsset("dwSpellVert.hlsl"));
        FShader dwFShader = *FShaderManager::Get()->Dereference(FShaderManager::Get()->GetAsset("dwSpellFrag.hlsl"));
        GeometryShader dwGShader = *GShaderManager::Get()->Dereference(GShaderManager::Get()->GetAsset("dwSpellGeo.hlsl"));

        Handle texture = TextureManager::Get()->GetAsset("spell.png");

        particleSys = GraphicsManager::Get()->CreateParticleSystem(1000, soVShader, soFShader, soGShader,
                                                                   dwVShader, dwFShader, dwGShader,
                                                                   texture);
    }

    void Terminate() override {
        if(particleSys) {
            GraphicsManager::Get()->DestroyParticleSystem(particleSys);
        }
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "player");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "player");
        ReadEndObject(t);
        Initialize();
    };
};

#endif

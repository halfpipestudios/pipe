#ifndef _PLAYER_CMP_H_
#define _PLAYER_CMP_H_

#include "sound.h"

struct PlayerCMP : CMP<PlayerCMP> {
    
    Sound jumpSound;

    void Initialize() {
        jumpSound.Initialize("jump.wav");
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

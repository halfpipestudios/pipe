#ifndef _ENEMY_CMP_
#define _ENEMY_CMP_

#include "sound.h"

struct EnemyCMP : CMP<EnemyCMP> {

    Sound3D sound;
    void Initialize() {
        sound.Initialize("orc.wav");
        sound.Play(true, 1.0f);
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "enemy");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "enemy");
        ReadEndObject(t);
        Initialize();
    };

};

#endif

#ifndef _ENEMY_CMP_
#define _ENEMY_CMP_

struct EnemyCMP : CMP<EnemyCMP> {
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "enemy");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "enemy");
        ReadEndObject(t);
    };

};

#endif

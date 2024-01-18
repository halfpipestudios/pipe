#ifndef _PLAYER_CMP_H_
#define _PLAYER_CMP_H_

struct PlayerCMP : CMP<PlayerCMP> {
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "player");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "player");
        ReadEndObject(t);
    };
};

#endif

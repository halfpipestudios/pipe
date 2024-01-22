#ifndef _MOVABLE_BOX_CMP_H_
#define _MOVABLE_BOX_CMP_H_

struct MovableBoxCMP : CMP<MovableBoxCMP> {
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "movable_box");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "movable_box");
        ReadEndObject(t);
    }

};

#endif

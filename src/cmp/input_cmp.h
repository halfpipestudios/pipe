#ifndef _INPUT_CMP_H_
#define _INPUT_CMP_H_

struct Input;

struct InputCMP : CMP<InputCMP> {
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "input");
        WriteEndObject(s);
    };
    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "input");
        ReadEndObject(t);
    };
};

#endif _INPUT_CMP_H_

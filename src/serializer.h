#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include "common.h"

struct Serializer {
    
    void Begin();
    void End(char *path);

    void WriteBytes(u8 *buffer, u32 size);

    void WriteCharacter(char character);
    void WriteString(char *str);
    void WriteInt(i32 number);
    void WriteReal(f32 number);
    
    u8 *data;
    u32 size;
    
    u32 tabOffset = 2;
    u32 curTabOffset = 0;

};

// TODO: Move tokenizer to this file
struct Tokenizer;

struct Serializable {
    virtual void Deserialize(Tokenizer *t) = 0;
    virtual void Serialize(Serializer *s) = 0;
    
    // NOTE: Serializer functions

    void Write(Serializer *s, char *name, f32 num);
    void Write(Serializer *s, char *name, i32 num);
    void Write(Serializer *s, char *name, u32 num);
    void Write(Serializer *s, char *name, char *str);
    void Write(Serializer *s, char *name, char c);

    void BeginObject(Serializer *s, char *name);
    void EndObject(Serializer *s);

    void BeginArray(Serializer *s, char *name);
    void EndArray(Serializer *s);

    // NOTE: Deserializer functions


private:
    void AdvanceTabs(Serializer *s);
};

#endif // _SERIALIZER_H_

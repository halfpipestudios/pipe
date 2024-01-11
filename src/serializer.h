#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include "common.h"
#include "tokenizer.h"
// TODO: Move tokenizer to this file

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

struct Serializable {
    virtual void Deserialize(Tokenizer *t) = 0;
    virtual void Serialize(Serializer *s) = 0;
    
    // NOTE: Serializer functions

    void Write(Serializer *s, char *name, f32 num);
    void Write(Serializer *s, char *name, i32 num);
    void Write(Serializer *s, char *name, char *str);

    void WriteBeginObject(Serializer *s, char *name);
    void WriteEndObject(Serializer *s);

    void WriteBeginArray(Serializer *s, char *name);
    void WriteEndArray(Serializer *s);

    // NOTE: Deserializer functions
    
    void Read(Tokenizer *t, char *name, f32 *num);
    void Read(Tokenizer *t, char *name, i32 *num);
    void Read(Tokenizer *t, char *name, char *str, u32 maxSize);

    void ReadBeginObject(Tokenizer *t, char *name);
    void ReadEndObject(Tokenizer *t);

    void ReadBeginArray(Tokenizer *t, char *name);
    void ReadEndArray(Tokenizer *t);

private:
    
    void AdvanceTabs(Serializer *s);
    void Expect(Tokenizer *t, Token *token , Token::Type type);
    void Error(Token *token, char *str...);

};

#endif // _SERIALIZER_H_

#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include "common.h"

struct Level;

#define MAX_DIGITS_FOR_INTEGER 20

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
    virtual void Serialize(Serializer *s) = 0;

    void Write(Serializer *s, char *name, f32 num);
    void Write(Serializer *s, char *name, i32 num);
    void Write(Serializer *s, char *name, u32 num);
    void Write(Serializer *s, char *name, char *str);
    void Write(Serializer *s, char *name, char c);

    void BeginObject(Serializer *s, char *name);
    void EndObject(Serializer *s);

    void BeginArray(Serializer *s, char *name);
    void EndArray(Serializer *s);

private:
    void AdvanceTabs(Serializer *s);
};

#endif // _SERIALIZER_H_

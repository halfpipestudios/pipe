#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include "common.h"

struct Level;

#define MAX_DIGITS_FOR_INTEGER 20

struct Serializer {
    
    void Begin();
    void End();

    void WriteBytes(u8 *buffer, u32 size);

    void WriteCharacter(char character);
    void WriteString(char *str);
    void WriteInt(i32 number);
    void WriteReal(f32 number);
    
    u8 *data;
    u32 size;

};

struct Serializable {
    virtual void Save() = 0;
    virtual void Load(char *path) = 0;

    void Begin();
    void End(char *filepath);
    
    void Write(char *name, f32 num);
    void Write(char *name, i32 num);
    void Write(char *name, char *str);
    void Write(char *name, char c);

    void BeginObject(char *name);
    void EndObject();

    void BeginArray(char *name);
    void EndArray();

    void AdvanceTabs();

    Serializer serializer;
    u32 tabOffset = 2;
    u32 curTabOffset = 0;
};

#endif // _SERIALIZER_H_

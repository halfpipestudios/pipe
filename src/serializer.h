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

#endif // _SERIALIZER_H_

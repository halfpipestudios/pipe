#include "serializer.h"
#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------
// Serializer Functions 
// -----------------------------------------------------------

void Serializer::Begin() {
    // NOTE: Must be at the start of this function
    MemoryManager::Get()->BeginTemporalMemory();
    data = (u8 *)MemoryManager::Get()->AllocTemporalMemory(0, 8);
    size = 0;
}

void Serializer::End() {

    WriteCharacter('\0');
    printf("%s\n", data);
    
    // TODO: Must be at the end of this functions
    data = nullptr;
    size = 0;
    MemoryManager::Get()->EndTemporalMemory();

}

void Serializer::WriteBytes(u8 *buffer, u32 size) {
    void *des = MemoryManager::Get()->AllocTemporalMemory(size, 1);
    memcpy(des, buffer, size);
    this->size += size;
}

void Serializer::WriteCharacter(char character) {
    WriteBytes((u8 *)&character, 1);
}

void Serializer::WriteString(char *str) {
    WriteBytes((u8 *)str, strlen(str));
}

void Serializer::WriteInt(i32 number) {
    static char buffer[4096];
    i32 len = snprintf(buffer, 4096, "%d", number);
    ASSERT(len >= 0);
    WriteBytes((u8 *)buffer, len);
}

void Serializer::WriteReal(f32 number) {
    static char buffer[4096];
    i32 len = snprintf(buffer, 4096, "%f", number);
    ASSERT(len >= 0);
    WriteBytes((u8 *)buffer, len);
}

// -----------------------------------------------------------
// Serializable functions 
// -----------------------------------------------------------

void Serializable::Write(Serializer *s, char *name, f32 num) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteReal(num);
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, i32 num) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteInt(num);
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, u32 num) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteInt(num);
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, char *str) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteCharacter('\"');
    s->WriteString(str);
    s->WriteCharacter('\"');
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, char c) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteCharacter('\'');
    s->WriteCharacter(c);
    s->WriteCharacter('\'');
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::BeginObject(Serializer *s, char *name) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": {\n");
    s->curTabOffset += s->tabOffset;
}

void Serializable::EndObject(Serializer *s) {
    s->curTabOffset -= s->tabOffset;
    AdvanceTabs(s);
    s->WriteString("}\n");
}

void Serializable::BeginArray(Serializer *s, char *name) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": [\n");
    s->curTabOffset += s->tabOffset;
}

void Serializable::EndArray(Serializer *s) {
    s->curTabOffset -= s->tabOffset;
    AdvanceTabs(s);
    s->WriteString("]\n");
}

void Serializable::AdvanceTabs(Serializer *s) {
    for(u32 i = 0; i < s->curTabOffset; ++i) {
        s->WriteCharacter(' ');
    }
}

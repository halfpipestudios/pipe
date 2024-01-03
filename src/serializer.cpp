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

void Serializable::Begin() {
    serializer.Begin();
}

void Serializable::End(char *filepath) {
    (void)filepath;
    serializer.End();
}

void Serializable::Write(char *name, f32 num) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": ");
    serializer.WriteReal(num);
    serializer.WriteCharacter('\n');
}

void Serializable::Write(char *name, i32 num) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": ");
    serializer.WriteInt(num);
    serializer.WriteCharacter('\n');
}

void Serializable::Write(char *name, char *str) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": ");
    serializer.WriteString(str);
    serializer.WriteCharacter('\n');
}

void Serializable::Write(char *name, char c) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": ");
    serializer.WriteCharacter(c);
    serializer.WriteCharacter('\n');
}

void Serializable::BeginObject(char *name) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": {\n");
    curTabOffset += tabOffset;
}

void Serializable::EndObject() {
    curTabOffset -= tabOffset;
    AdvanceTabs();
    serializer.WriteString("}\n");
}

void Serializable::BeginArray(char *name) {
    AdvanceTabs();
    serializer.WriteString(name);
    serializer.WriteString(": [\n");
    curTabOffset += tabOffset;
}

void Serializable::EndArray() {
    curTabOffset -= tabOffset;
    AdvanceTabs();
    serializer.WriteString("]\n");
}

void Serializable::AdvanceTabs() {
    for(u32 i = 0; i < curTabOffset; ++i) {
        serializer.WriteCharacter(' ');
    }
}

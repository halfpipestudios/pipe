#include "serializer.h"
#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


// -----------------------------------------------------------
// Serializer Functions 
// -----------------------------------------------------------

void Serializer::Begin() {
    // NOTE: Must be at the start of this function
    MemoryManager::Get()->BeginTemporalMemory();
    data = (u8 *)MemoryManager::Get()->AllocTemporalMemory(0, 8);
    size = 0;
}

void Serializer::End(char *path) {

    FILE *file = fopen(path, "w");
    fwrite(data, size, 1, file);
    fclose(file);
    
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

void Serializer::WriteUInt(u32 number) {
    static char buffer[4096];
    i32 len = snprintf(buffer, 4096, "%u", number);
    ASSERT(len >= 0);
    WriteBytes((u8 *)buffer, len);
}

void Serializer::WriteUInt64(u64 number) {
    static char buffer[4096];
    i32 len = snprintf(buffer, 4096, "%llu", number);
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

void Serializable::AdvanceTabs(Serializer *s) {
    for(u32 i = 0; i < s->curTabOffset; ++i) {
        s->WriteCharacter(' ');
    }
}

// NOTE: Serializer functions

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
    s->WriteUInt(num);
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, u64 num) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteUInt64(num);
    s->WriteCharacter(',');
    s->WriteCharacter('\n');
}

void Serializable::Write(Serializer *s, char *name, bool val) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": ");
    s->WriteInt((i32)val);
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

void Serializable::Write(Serializer *s, char *name, Vec3 vec) {
    WriteBeginObject(s, name);
    Write(s, "x", vec.x);
    Write(s, "y", vec.y);
    Write(s, "z", vec.z);
    WriteEndObject(s); 
}

void Serializable::WriteBeginObject(Serializer *s, char *name) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": {\n");
    s->curTabOffset += s->tabOffset;
}

void Serializable::WriteEndObject(Serializer *s) {
    s->curTabOffset -= s->tabOffset;
    AdvanceTabs(s);
    s->WriteString("}\n");
}

void Serializable::WriteBeginArray(Serializer *s, char *name) {
    AdvanceTabs(s);
    s->WriteString(name);
    s->WriteString(": [\n");
    s->curTabOffset += s->tabOffset;
}

void Serializable::WriteEndArray(Serializer *s) {
    s->curTabOffset -= s->tabOffset;
    AdvanceTabs(s);
    s->WriteString("]\n");
}

// NOTE: Deserializer functions

void Serializable::Expect(Tokenizer *t, Token *token, Token::Type type) {
    t->NextToken(token);
    if(token->type != type) {
        Token tmpToken;
        tmpToken.type = type;
        Error(token, "Expecting %s but get %s(%.*s) instead", tmpToken.TypeToString(), token->TypeToString(), (u32)(token->end - token->start), token->start);
    }
}

void Serializable::Error(Token *token, char *str...) {
    static char errorStr[256];
    va_list args;
    va_start(args, str);
    vsnprintf(errorStr, 256, str, args);
    va_end(args);
    printf("line:%d:col:%d: error: %s\n", token->line, token->col, errorStr);
    ASSERT(!"SERIALIZER ERROR");
}

void Serializable::Read(Tokenizer *t, char *name, f32 *num) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::REAL);
    *num = token.fValue;
    Expect(t, &token, Token::Type::COMMA);
}

void Serializable::Read(Tokenizer *t, char *name, i32 *num) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::INTEGER);
    *num = (i32)token.iValue;
    Expect(t, &token, Token::Type::COMMA);
}

void Serializable::Read(Tokenizer *t, char *name, u32 *num) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::INTEGER);
    *num = (u32)token.uValue;
    Expect(t, &token, Token::Type::COMMA);
}

void Serializable::Read(Tokenizer *t, char *name, u64 *num) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::INTEGER);
    *num = token.uValue;
    Expect(t, &token, Token::Type::COMMA);
}


void Serializable::Read(Tokenizer *t, char *name, bool *val) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::INTEGER);
    *val = token.iValue != 0;
    Expect(t, &token, Token::Type::COMMA);
}

void Serializable::Read(Tokenizer *t, char *name, char *str, u32 maxSize) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::STRING);
    u32 size = MIN(token.end - token.start - 2, maxSize-1);
    memcpy(str, token.start+1, size);
    str[size] = '\0';
    Expect(t, &token, Token::Type::COMMA);
}


void Serializable::Read(Tokenizer *t, char *name, Vec3 *vec) {
    ReadBeginObject(t, name);
    Read(t, "x", &vec->x);
    Read(t, "y", &vec->y);
    Read(t, "z", &vec->z);
    ReadEndObject(t);
}

void Serializable::ReadBeginObject(Tokenizer *t, char *name) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::L_BRACE);
}

void Serializable::ReadEndObject(Tokenizer *t) {
    Token token = {};
    Expect(t, &token, Token::Type::R_BRACE);
}

void Serializable::ReadBeginArray(Tokenizer *t, char *name) {
    Token token = {};
    Expect(t, &token, Token::Type::IDENTIFIER);
    if(!token.Contains(name)) Error(&token, "Expecting identifier named: %s", name);
    Expect(t, &token, Token::Type::DOUBLE_DOT);
    Expect(t, &token, Token::Type::L_SQUARE_BRACKET);
}

void Serializable::ReadEndArray(Tokenizer *t) {
    Token token = {};
    Expect(t, &token, Token::Type::R_SQUARE_BRACKET);
}

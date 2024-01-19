#include "tokenizer.h"
#include "platform_manager.h"
#include "memory_manager.h"

#include <stdlib.h>

char *Token::TypeToString() {
    switch(type) {
    case Type::UNKNOW:     return "UNKNOW";
    case Type::INTEGER:    return "INTEGER";
    case Type::REAL:       return "REAL";
    case Type::STRING:     return "STRING";
    case Type::IDENTIFIER: return "IDENTIFIER";

    case Type::L_BRACE: return "L_BRACE";
    case Type::R_BRACE: return "R_BRACE";

    case Type::L_SQUARE_BRACKET: return "L_SQUARE_BRACKET"; 
    case Type::R_SQUARE_BRACKET: return "R_SQUARE_BRACKET";

    case Type::DOUBLE_DOT: return "DOUBLE_DOT";
    case Type::COMMA: return "COMMA";
    }

    ASSERT(!"Invalid code path");
    return nullptr;
}

bool Token::Contains(char *str) {
    u32 len = end - start;
    if(len != strlen(str)) return false;
    for(u32 i = 0; i < len; ++i) {
        if(start[i] != str[i]) return false;
    }
    return true;
}

void Tokenizer::Begin(char *filepath) {
    MemoryManager::Get()->BeginTemporalMemory();
    
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(filepath);
    
    start = (char *)file.data;
    end = start + file.size;
    current = start;

    currentCol = TOKENIZER_START_COL_AND_LINE;
    currentLin = TOKENIZER_START_COL_AND_LINE;
}

void Tokenizer::End() {

    start = nullptr;
    end = nullptr;
    current = nullptr;

    MemoryManager::Get()->EndTemporalMemory();
}

bool Tokenizer::NextToken(Token *token) {
    
    SkipSpaceAndNewLine();

    if(IsEnd()) return false;
    
    SetTokenColAndLine(token);

    switch(*current) {

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '0':
    case '.': case '-': {
      TokenizeNumber(token);
    } break;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
    case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
    case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_': {
        TokenizeIdentifier(token);
    } break;
    
    case '"': {
        TokenizeString(token);
    } break;

    case '{': {
        TokenizeSingleCharacter(token, Token::Type::L_BRACE);
    } break;
    
    case '}': {
        TokenizeSingleCharacter(token, Token::Type::R_BRACE);
    } break;

    case '[': {
        TokenizeSingleCharacter(token, Token::Type::L_SQUARE_BRACKET);
    } break;

    case ']': {
        TokenizeSingleCharacter(token, Token::Type::R_SQUARE_BRACKET);
    } break;
    
    case ':': {
        TokenizeSingleCharacter(token, Token::Type::DOUBLE_DOT);
    } break;

    case ',': {
        TokenizeSingleCharacter(token, Token::Type::COMMA);
    } break;

    default: {
        TokenizeUnknow(token);
    } break;
     
    };

    return true;
}

void Tokenizer::SkipSpaceAndNewLine() {
    while(IsSpace() && !IsEnd()) {
        if(*current == '\n') {
            ++currentLin;
            currentCol = TOKENIZER_START_COL_AND_LINE;
        }
        AdvanceCurrent();
    }
}

void Tokenizer::TokenizeNumber(Token *token) {
    
    bool neg = false;
    if(*current == '-') {
        neg = true;
        ++current;
    }
    
    if(IsCurrentTokenNumberReal()) {
        TokenizeReal(token);
        if(neg) token->fValue = -token->fValue;
    } else {
        TokenizeInteger(token);
        if(neg) token->iValue = -token->iValue;
    }
}

void Tokenizer::TokenizeReal(Token *token) {
    token->type = Token::Type::REAL;
    token->start = current;
    while(!IsEnd() && IsDigit()) {
        AdvanceCurrent();
    }
    ASSERT(*current == '.'); AdvanceCurrent();
    while(!IsEnd() && IsDigit()) {
        AdvanceCurrent();
    }
    token->end = current;
    
    MemoryManager::Get()->BeginTemporalMemory();
    char *buffer = TemporalNullTerminatedTokenContent(token);
    token->fValue = atof(buffer);
    MemoryManager::Get()->EndTemporalMemory();
}

void Tokenizer::TokenizeInteger(Token *token) {
    token->type = Token::Type::INTEGER;
    token->start = current;
    while(!IsEnd() && IsDigit()) {
        AdvanceCurrent();
    }
    token->end = current;
    
    MemoryManager::Get()->BeginTemporalMemory();
    char *buffer = TemporalNullTerminatedTokenContent(token);
    token->iValue = atoll(buffer);
    MemoryManager::Get()->EndTemporalMemory();
}

void Tokenizer::TokenizeIdentifier(Token *token) {
    token->type = Token::Type::IDENTIFIER;
    token->start = current;
    while(!IsEnd() && (IsAlpha() || IsDigit() || *current == '_')) {
        AdvanceCurrent();
    }
    token->end = current;
    token->iValue = token->end - token->start;
}

void Tokenizer::TokenizeString(Token *token) {
    token->type = Token::Type::STRING;
    token->start = current;
    ASSERT(*current == '"'); AdvanceCurrent();
    while(!IsEnd() && *current != '"') {
        AdvanceCurrent();
    }
    ASSERT(*current == '"'); AdvanceCurrent();
    token->end = current;
    token->iValue = token->end - token->start;
}

void Tokenizer::TokenizeSingleCharacter(Token *token, Token::Type type) {
    token->type = type;
    token->start = current;
    AdvanceCurrent();
    token->end = current;
    token->iValue = 1;
}

void Tokenizer::TokenizeUnknow(Token *token) {
    token->type = Token::Type::UNKNOW;
    token->start = current;
    AdvanceCurrent();
    token->end = current;
    token->iValue = 1;
}

bool Tokenizer::IsCurrentTokenNumberReal() {
    char *tmpCurrent = current;
    while((tmpCurrent != end) && (*tmpCurrent >= '0' && *tmpCurrent <= '9')) {
        ++tmpCurrent;
    }
    return (tmpCurrent != end) && (*tmpCurrent == '.');
}

char *Tokenizer::TemporalNullTerminatedTokenContent(Token *token) {
    u32 bufferSize = (token->end - token->start); 
    char *buffer = (char *)MemoryManager::Get()->AllocTemporalMemory(bufferSize + 1, 8); 
    memcpy(buffer, token->start, bufferSize);
    buffer[bufferSize] = '\0';
    return buffer;
}

void Tokenizer::SetTokenColAndLine(Token *token) {
    token->col = currentCol;
    token->line = currentLin;
}

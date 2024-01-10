#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"

struct Token {
    
    enum class Type {
        
        UNKNOW,

        INTEGER,
        REAL,
        STRING,
        IDENTIFIER,

        L_BRACE,
        R_BRACE,

        L_SQUARE_BRACKET,
        R_SQUARE_BRACKET,

        DOUBLE_DOT,
        COMMA,
        
    };

    Type type;
    char *start;
    char *end;
    u32 col;
    u32 line;

    union {
        i32 iValue;
        u32 uValue;
        f32 fValue;
    };

    char *TypeToString();
};

#define TOKENIZER_START_COL_AND_LINE 1

struct Tokenizer {
    
    char *start;
    char *end;
    char *current;

    u32 currentLin;
    u32 currentCol;

    void Begin(char *filepath);
    void End();
    
    bool NextToken(Token *token);

private:
    
    inline bool IsSpace() { char c = *current; return (c == ' ' || c == '\n' || c == '\r' || c == '\t');  }
    inline bool IsDigit() { char c = *current; return (c >= '0' && c <= '9'); }
    inline bool IsAlpha() { char c = *current; return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')); }
    inline bool IsEnd() { return current == end; }
    inline void AdvanceCurrent() { ++current; ++currentCol; }

    
    bool IsCurrentTokenNumberReal();
    char *TemporalNullTerminatedTokenContent(Token *token);
    void SetTokenColAndLine(Token *token);

    void SkipSpaceAndNewLine();
    
    void TokenizeNumber(Token *token); 
    void TokenizeReal(Token *token);
    void TokenizeInteger(Token *token);
    void TokenizeIdentifier(Token *token);
    void TokenizeString(Token *token);
    void TokenizeSingleCharacter(Token *token, Token::Type type);
    void TokenizeUnknow(Token *token);

};

#endif // _TOKENIZER_H_

#ifndef __INC_TOKEN_H__
#define __INC_TOKEN_H__

#include "stddef.h"

typedef enum {
    TOKEN_NUMBER,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_CARET,
    TOKEN_EOF
}TokenType;

typedef struct {
    TokenType type;
    double value;
    // void * for any?
} Token;

typedef struct {
    const char * input;
    size_t pos;
    char current;
} Lexer;

Token lexer_next(Lexer * lexer);

#endif // __INC_TOKEN_H__

#ifndef __INC_LEXER_H__
#define __INC_LEXER_H__

#include "stddef.h"
#include "vars.h"
#include "tokens.h"

typedef struct {
    TokenType type;
    Value value;
    char * ident; // names
} Token;

typedef struct {
    const char * input;
    size_t pos;
    int line_start;
    int indent_stack[64];
    int indent_top;
    int pending_indents;
    int pending_dedents;
} Lexer;


void lexer_init(Lexer * lexer, const char * input);
Token lexer_next(Lexer * lexer);

#endif // __INC_LEXER_H__

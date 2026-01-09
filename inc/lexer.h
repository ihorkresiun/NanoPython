#ifndef __INC_LEXER_H__
#define __INC_LEXER_H__

#include "stddef.h"

typedef enum {
    TOKEN_EOF,

    TOKEN_NUMBER,
    TOKEN_IDENT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_CARET,
    TOKEN_ASSIGN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DEF,
    TOKEN_COLON,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_PRINT,
    TOKEN_PRINT_DBG,
}TokenType;

typedef struct Ast Ast; // forward declaration
typedef struct Scope Scope; // forward declaration

typedef struct Function {
    char** params;
    int param_count;
    Ast* body;
    Scope* closure;
}Function;

typedef enum {
    VAL_NUMBER,
    VAL_BOOL,
    VAL_NONE,
    VAL_FUNCTION
}ValueType;

typedef struct Value {
    ValueType type;
    union {
        double number;
        int boolean;
        struct Function* function;
    }value;
} Value;

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

Token lexer_next(Lexer * lexer);

#endif // __INC_LEXER_H__

#ifndef __INC_LEXER_H__
#define __INC_LEXER_H__

#include "stddef.h"

typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_CARET,
    TOKEN_ASSIGN, // =
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FUNCDEF,
    TOKEN_COLON,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_EMPTY,
    TOKEN_EOF
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
    char current;
} Lexer;

Token lexer_next(Lexer * lexer);

#endif // __INC_LEXER_H__

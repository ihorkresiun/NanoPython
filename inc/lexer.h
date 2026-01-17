#ifndef __INC_LEXER_H__
#define __INC_LEXER_H__

#include "stddef.h"

typedef enum {
    TOKEN_EOF,

    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_CARET,
    TOKEN_ASSIGN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DEF,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
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
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT
}TokenType;

typedef struct Ast Ast; // forward declaration
typedef struct Scope Scope; // forward declaration

typedef struct Function {
    char* name;
    char** params;
    int param_count;
    Ast* body;
    Scope* scope; // Closure scope
}Function;

typedef enum {
    VAL_NUMBER,
    VAL_BOOL,
    VAL_LIST,
    VAL_FUNCTION,
    VAL_STRING,
    VAL_NONE,
}ValueType;

typedef struct List List; // forward declaration

typedef struct Value {
    ValueType type;
    union {
        double number;
        int boolean;
        char* string;
        List* list;
        struct Function* function;
    }value;
} Value;

typedef struct List {
    int count;
    int capacity;
    Value* items;
} List;

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

// Peek at the next token without consuming it
Token lexer_peek_next(Lexer* lexer);

#endif // __INC_LEXER_H__

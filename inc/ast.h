#ifndef __INC_AST_H__
#define __INC_AST_H__

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_BINARY,
    AST_VAR,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_BLOCK,
    AST_FUNCDEF,
    AST_CALL,
    AST_RETURN
} AstType;

typedef struct Ast;

struct AstIf {
    struct Ast* condition;
    struct Ast* then_branch;
    struct Ast* else_branch;
}AstIf;

struct AstWhile {
    struct Ast* condition;
    struct Ast* body;
}AstWhile;

struct AstBlock {
    struct Ast** statements;
    int count;
}AstBlock;

struct AstFuncDef {
    char* name;
    char** params;
    int param_count;
    struct Ast* body;
}AstFuncDef;

struct AstCall{
    char* name;
    struct Ast** args;
    int arg_count;
}AstCall;

struct AstReturn{
    struct Ast* value;
}AstReturn;

typedef struct Ast {
    AstType type;
    union {
        struct  {
            double value;
        }Number;

        struct  {
            struct Ast* left;
            struct Ast* right;
            TokenType op;
        }Binary;

        struct  {
            char* name;
        }Variable;

        struct  {
            char* name;
            struct Ast* value;
        }Assign;

        AstIf If;
        AstWhile While;
        AstBlock Block;
        AstFuncDef Func;

    };
} Ast;

Ast* ast_new_number(double value);
Ast* ast_new_expr(TokenType type, Ast* left, Ast* right);
Ast* ast_new_var(const char* name);
Ast* ast_new_assign(const char* name, Ast* value);

void ast_free(Ast* node);

#endif // __INC_AST_H__
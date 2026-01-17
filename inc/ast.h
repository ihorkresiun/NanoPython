#ifndef __INC_AST_H__
#define __INC_AST_H__

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_STRING,
    AST_BINARY,
    AST_UNARY,
    AST_VAR,
    AST_ASSIGN,
    AST_IF,
    AST_ELSE,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_LIST,
    AST_INDEX,
    AST_ASSIGN_INDEX,
    AST_FUNCDEF,
    AST_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_PRINT
} AstType;


typedef struct Ast {
    AstType type;
    union {
        struct  {
            double value;
        }Number;

        struct  {
            struct Ast** elements;
            int count;
        }List;

        struct {
            Ast* target;
            Ast* index;
        }Index;

        struct {
            Ast* target;
            Ast* index;
            Ast* value;
        }AssignIndex;

        struct  {
            char* value;
        }String;

        struct  {
            struct Ast* left;
            struct Ast* right;
            TokenType op;
        }Binary;

        struct  {
            struct Ast* value;
            TokenType op;
        }Unary;

        struct  {
            char* name;
        }Variable;

        struct  {
            char* name;
            struct Ast* value;
        }Assign;

        struct {
            struct Ast* condition;
            struct Ast* then_branch;
            struct Ast* else_branch;
        }If;

        struct {
            struct Ast* condition;
            struct Ast* body;
        }While;

        struct {
            const char* var;
            Ast* iterable;
            Ast* body;
        }For;

        struct {
            struct Ast** statements;
            int count;
        }Block;

        struct {
            char* name;
            char** args;
            int argc;
            struct Ast* body;
        }FuncDef;

        struct {
            char* name;
            struct Ast** args;
            int argc;
        }Call;

        struct {
            struct Ast* value;
        }Return;

        struct {
            Ast* expr;
        }Print;
    };
} Ast;

Ast* ast_new_number(double value);
Ast* ast_new_string(const char* value);
Ast* ast_new_binary_expr(TokenType type, Ast* left, Ast* right);
Ast* ast_new_unary_expr(TokenType type, Ast* value);
Ast* ast_new_var(const char* name);
Ast* ast_new_assign(const char* name, Ast* value);
Ast* ast_new_if(Ast* condition, Ast* then_block, Ast* else_block);
Ast* ast_new_while(Ast* condition, Ast* body);
Ast* ast_new_for(const char* var, Ast* iterable, Ast* body);
Ast* ast_new_block(Ast** statements, int count);
Ast* ast_new_list(Ast** elements, int count);
Ast* ast_new_index(Ast* target, Ast* index);
Ast* ast_new_assign_index(Ast* target, Ast* index, Ast* value);
Ast* ast_new_funcdef(const char* name, char** args, int argc, Ast* body);
Ast* ast_new_call(const char* name, Ast** args, int argc);
Ast* ast_new_return(Ast* value);
Ast* ast_new_break();
Ast* ast_new_continue();
Ast* ast_new_print(Ast* expr);

void ast_free(Ast* node);

#endif // __INC_AST_H__
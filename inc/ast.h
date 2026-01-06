#ifndef __INC_AST_H__
#define __INC_AST_H__

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_BINARY,
    AST_VAR,
    AST_ASSIGN
} AstType;


typedef struct Ast {
    AstType type;
    double value; // Used if type is AST_NUMBER
    struct Ast* left;  // Used if type is AST_BINARY
    struct Ast* right; // Used if type is AST_BINARY
    TokenType op;          // Used if type is AST_BINARY
    char* name;       // Used if type is AST_VARIABLE
} Ast;

Ast* ast_new_number(double value);
Ast* ast_new_expr(TokenType type, Ast* left, Ast* right);
Ast* ast_new_var(const char* name);
Ast* ast_new_assign(const char* name, Ast* value);

void ast_free(Ast* node);

#endif // __INC_AST_H__
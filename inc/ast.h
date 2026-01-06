#ifndef __INC_AST_H__
#define __INC_AST_H__

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_BINARY,
} AstType;


typedef struct Ast {
    AstType type;
    double value; // Used if type is AST_NUMBER
    struct Ast* left;  // Used if type is AST_BINARY
    struct Ast* right; // Used if type is AST_BINARY
    TokenType op;          // Used if type is AST_BINARY
} Ast;

Ast* ast_new_number(double value);
Ast* ast_new_expr(TokenType type, Ast* left, Ast* right);
void ast_free(Ast* node);

#endif // __INC_AST_H__
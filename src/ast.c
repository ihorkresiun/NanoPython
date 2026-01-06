#include "ast.h"

#include "stdlib.h"
#include "string.h"


Ast* ast_new_number(double value) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_NUMBER;
    n->Number.value = value;
    return n;
}

Ast* ast_new_expr(TokenType type, Ast* left, Ast* right) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_BINARY;
    n->Binary.left = left;
    n->Binary.right = right;
    n->Binary.op = type;
    return n;
}

Ast* ast_new_var(const char* name) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_VAR;
    n->Variable.name = strdup(name);
    return n;
}

Ast* ast_new_assign(const char* name, Ast* value) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_ASSIGN;
    n->Assign.name = strdup(name);
    n->Assign.value = value;
    return n;
}

void ast_free(Ast* node) {
    if (node == NULL) return;

    if (node->type == AST_BINARY) {
        ast_free(node->Binary.left);
        ast_free(node->Binary.right);
    }

    free(node);
}
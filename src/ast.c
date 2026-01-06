#include "ast.h"

#include "stdlib.h"
#include "string.h"


Ast* ast_new_number(double value) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_NUMBER;
    n->value = value;
    return n;
}

Ast* ast_new_expr(TokenType type, Ast* left, Ast* right) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_BINARY;
    n->left = left;
    n->right = right;
    n->op = type;
    return n;
}

Ast* ast_new_var(const char* name) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_VAR;
    n->name = strdup(name);
    return n;
}

Ast* ast_new_assign(const char* name, Ast* value) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_ASSIGN;
    n->name = strdup(name);
    n->left = value;
    return n;
}

void ast_free(Ast* node) {
    if (node == NULL) return;

    if (node->type == AST_BINARY) {
        ast_free(node->left);
        ast_free(node->right);
    }

    free(node);
}
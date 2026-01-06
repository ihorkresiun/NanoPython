#include "ast.h"

#include "stdlib.h"

Ast* ast_new_number(double value) {
    Ast* n = malloc(sizeof(Ast));
    n->type = AST_NUMBER;
    n->value = value;
    return n;
}

Ast* ast_new_expr(TokenType type, Ast* left, Ast* right) {
    Ast* n = malloc(sizeof(Ast));
    n->type = type;
    n->left = left;
    n->right = right;
    n->op = type;
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
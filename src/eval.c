#include "eval.h"

#include "ast.h"
#include "token.h"

#include "stdio.h"
#include "stdlib.h"

double eval(Ast* node) {
    if (node->type == AST_NUMBER) {
        return node->value;
    }

    double left = eval(node->left);
    double right = eval(node->right);

    switch (node->op) {
        case TOKEN_PLUS:  return left + right;
        case TOKEN_MINUS: return left - right;
        case TOKEN_STAR:  return left * right;
        case TOKEN_SLASH: return left / right;
        default:
            printf("Unknown operator\n");
            exit(1);
    }
}
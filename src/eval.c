#include "eval.h"
#include "token.h"

double eval(Expr* e)
{
    switch(e->type) {
        case EXPR_NUMBER: return e->value;
        case EXPR_ADD: return eval(e->left) + eval(e->right);
        case EXPR_SUB: return eval(e->left) - eval(e->right);
        case EXPR_MUL: return eval(e->left) * eval(e->right);
        case EXPR_DIV: return eval(e->left) / eval(e->right);
    }

    return 0;
}

#include "eval.h"

#include "ast.h"
#include "lexer.h"
#include "vars.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

double eval(Ast* node) {
    switch(node->type) {
        case AST_NUMBER:
            return node->Number.value;
        break;
        case AST_VAR: {
            Var* v = var_find(node->Variable.name);
            if (!v) {
                printf("Undefined variable: %s\n", node->Variable.name);
                exit(1);
            }
            return v->value;
        }
        break;
        case AST_ASSIGN: {
            double value = eval(node->Assign.value);
            var_set(node->Assign.name, value);
            return value;
        }
        break;
        case AST_BINARY: {
            double left = eval(node->Binary.left);
            double right = eval(node->Binary.right);

            switch (node->Binary.op) {
                case TOKEN_PLUS:  return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_STAR:  return left * right;
                case TOKEN_SLASH: return left / right;
                case TOKEN_CARET: return pow(left, right);
                default:
                    printf("Unknown operator\n");
                    exit(1);
            }
        }
        break;
        default:
            printf("Unknown AST node type\n");
            exit(1);
    }
}
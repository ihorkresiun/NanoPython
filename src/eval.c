#include "eval.h"

#include "ast.h"
#include "lexer.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

static int is_true(Value v) {
    switch (v.type) {
        case VAL_BOOL:      return v.value.boolean;
        case VAL_NUMBER:    return v.value.number != 0;
        case VAL_NONE:      return 0;
        default: return 1;
    }
}

Value eval(Ast* node, Scope* scope) {
    switch(node->type) {
        case AST_NUMBER:
            return make_number(node->Number.value);
        break;
        case AST_VAR: {
            Var* v = scope_find(scope, node->Variable.name);
            if (!v) {
                printf("Undefined variable: %s\n", node->Variable.name);
                exit(1);
            }
            return v->value;
        }
        break;
        case AST_ASSIGN: {
            Value value = eval(node->Assign.value, scope);
            scope_set(scope, node->Assign.name, value);
            return value;
        }
        break;
        case AST_BINARY: {
            Value left = eval(node->Binary.left, scope);
            Value right = eval(node->Binary.right, scope);

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

        case AST_IF: {
            Value cond = eval(node->If.condition, scope);
            if (is_true(cond)) {
                return eval(node->If.then_branch, scope)
            } else if (node->If.else_branch) {
                return eval(node->If.else_branch, scope);
            }
            return make_none();
        }
        break;

        case AST_ELSE: {

        }

        case AST_WHILE: {
            while (is_true(eval(node->While.condition, scope))) {
                eval(node->While.body, scope);
            }
            return make_none();
        }

        case AST_FUNCDEF: {
            Function* fn = malloc(sizeof(Function))
            
        }

        default:
            printf("Unknown AST node type\n");
            exit(1);
    }
}
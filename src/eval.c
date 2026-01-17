#include "eval.h"

#include "ast.h"
#include "lexer.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

static int is_true(Value v) {
    switch (v.type) {
        case VAL_BOOL:      return v.value.boolean;
        case VAL_NUMBER:    return v.value.number != 0;
        case VAL_NONE:      return 0;
        default: return 1;
    }
}

EvalResult eval(Ast* node, Scope* scope) {
    switch(node->type) {
        case AST_NUMBER:
            return (EvalResult){make_number(node->Number.value), NORMAL};
        break;

        case AST_VAR: {
            Var* v = scope_find(scope, node->Variable.name);
            if (!v) {
                printf("Undefined variable: %s\n", node->Variable.name);
                exit(1);
            }
            // printf ("Scope '%s', Variable: %s = %g\n", scope->name, node->Variable.name, v->value.value.number);
            return (EvalResult){v->value, NORMAL};
        }
        break;

        case AST_STRING:
            return (EvalResult){make_string(node->String.value), NORMAL};
        break;
        
        case AST_ASSIGN: {
            EvalResult res = eval(node->Assign.value, scope);
            // printf ("Scope '%s', Assign: %s = %g\n", scope->name, node->Assign.name, value.value.number);
            scope_set(scope, node->Assign.name, res.value);
            return res;
        }
        break;

        case AST_BINARY: {
            Value left = (eval(node->Binary.left, scope)).value;
            Value right = (eval(node->Binary.right, scope)).value;

            switch (node->Binary.op) {
                case TOKEN_PLUS:
                    if (left.type == VAL_STRING && right.type == VAL_STRING) {
                        char* combined = malloc(strlen(left.value.string) + strlen(right.value.string) + 1);
                        strcpy(combined, left.value.string);
                        strcat(combined, right.value.string);
                        Value v = make_string(combined);
                        free(combined);
                        return (EvalResult){v, NORMAL};
                    } else if (left.type == VAL_STRING && right.type == VAL_NUMBER) {
                        char buffer[64];
                        sprintf(buffer, "%s%g", left.value.string, right.value.number);
                        return (EvalResult){make_string(buffer), NORMAL};
                    } else if (left.type == VAL_NUMBER && right.type == VAL_STRING) {
                        char buffer[64];
                        sprintf(buffer, "%g%s", left.value.number, right.value.string);
                        return (EvalResult){make_string(buffer), NORMAL};
                    }
                    
                    return (EvalResult){make_number(left.value.number + right.value.number), NORMAL};
                break;

                case TOKEN_MINUS: return (EvalResult){make_number(left.value.number - right.value.number), NORMAL};
                case TOKEN_STAR:  return (EvalResult){make_number(left.value.number * right.value.number), NORMAL};
                case TOKEN_SLASH: return (EvalResult){make_number(left.value.number / right.value.number), NORMAL};
                case TOKEN_CARET: return (EvalResult){make_number(pow(left.value.number, right.value.number)), NORMAL};
                case TOKEN_LT:    return (EvalResult){make_bool(left.value.number < right.value.number), NORMAL};
                case TOKEN_GT:    return (EvalResult){make_bool(left.value.number > right.value.number), NORMAL};
                case TOKEN_LE:    return (EvalResult){make_bool(left.value.number <= right.value.number), NORMAL};
                case TOKEN_GE:    return (EvalResult){make_bool(left.value.number >= right.value.number), NORMAL};
                case TOKEN_EQ:    return (EvalResult){make_bool(left.value.number == right.value.number), NORMAL};
                case TOKEN_NE:    return (EvalResult){make_bool(left.value.number != right.value.number), NORMAL};
                case TOKEN_AND:   return (EvalResult){make_bool(is_true(left) && is_true(right)), NORMAL};
                case TOKEN_OR:    return (EvalResult){make_bool(is_true(left) || is_true(right)), NORMAL};
                
                default:
                    printf("Unknown operator\n");
                    exit(1);
            }
        }
        break;

        case AST_UNARY: {
            Value val = (eval(node->Unary.value, scope)).value;
            switch (node->Unary.op) {
                case TOKEN_MINUS:
                    return (EvalResult){make_number(-val.value.number), NORMAL};
                case TOKEN_NOT:
                    return (EvalResult){make_bool(!is_true(val)), NORMAL};
                default:
                    printf("Unknown unary operator\n");
                    exit(1);
            }
        }
        break;

        case AST_IF: {
            Value cond = (eval(node->If.condition, scope)).value;
            if (is_true(cond)) {
                return eval(node->If.then_branch, scope);
            } else if (node->If.else_branch) {
                return eval(node->If.else_branch, scope);
            }
            return (EvalResult){make_none(), NORMAL};
        }
        break;

        case AST_WHILE: {
            EvalResult result = {make_none(), NORMAL};
            while (is_true((eval(node->While.condition, scope)).value)) {
                result = eval(node->While.body, scope);
                if (result.status == BREAKING) {
                    break;
                } else if (result.status == CONTINUING) {
                    continue;
                } else if (result.status == RETURNING) {
                    return result;
                }
            }
            return result;
        }
        break;

        case AST_BLOCK: {
            EvalResult result = {make_none(), NORMAL};
            for (int i = 0; i < node->Block.count; i++) {
                result = eval(node->Block.statements[i], scope);
                if (result.status != NORMAL) {
                    return result;
                }
            }
            return result;
        }
        break;

        case AST_FUNCDEF: {
            Function* fn = malloc(sizeof(Function));
            fn->name = strdup(node->FuncDef.name);
            fn->params = node->FuncDef.args;
            fn->param_count = node->FuncDef.argc;
            fn->body = node->FuncDef.body;
            fn->scope = scope;

            Value v;
            v.type = VAL_FUNCTION;
            v.value.function = fn;
            scope_set(scope, node->FuncDef.name, v);
            return (EvalResult){make_none(), NORMAL};
        }
        break;

        case AST_CALL: {
            Var* v = scope_find(scope, node->Call.name);
            if (!v || v->value.type != VAL_FUNCTION) {
                printf("Undefined function: %s\n", node->Call.name);
                exit(1);
            }

            Function* fn = v->value.value.function;
            if (node->Call.argc != fn->param_count) {
                printf("Function %s expects %d arguments, got %d\n", node->Call.name, fn->param_count, node->Call.argc);
                exit(1);
            }

            Scope fn_scope;
            fn_scope.name = fn->name;
            fn_scope.vars = NULL;
            fn_scope.parent = scope;
            
            for (int i = 0; i < fn->param_count; i++) {
                Value arg_value = (eval(node->Call.args[i], scope)).value;
                scope_set(&fn_scope, fn->params[i], arg_value);
            }

            return eval(fn->body, &fn_scope);
        }
        break;

        case AST_RETURN: {
            Value v = make_none();
            if (node->Return.value) {
                v = (eval(node->Return.value, scope)).value;
            }
            scope->return_value = v;
            return (EvalResult){v, RETURNING};
        }
        break;

        case AST_BREAK: {
            return (EvalResult){make_none(), BREAKING};
        }
        break;

        case AST_CONTINUE: {
            return (EvalResult){make_none(), CONTINUING};
        }
        break;

        case AST_PRINT: {
            Value val = (eval(node->Print.expr, scope)).value;
            if (val.type == VAL_NUMBER) {
                printf("%g\n", val.value.number);
            } else if (val.type == VAL_STRING) {
                printf("%s\n", val.value.string);
            } else if (val.type == VAL_BOOL) {
                printf("%s\n", val.value.boolean ? "True" : "False" );
            } else if (val.type == VAL_NONE) {
                printf("=\n");
            } else if (val.type == VAL_FUNCTION) {
                printf("<function>\n");
            }
            return (EvalResult){make_none(), NORMAL};
        }
        break;

        default:
            printf("Unknown AST node type\n");
            exit(1);
    }
}
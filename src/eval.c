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

static Value binary_op(Value left, Value right, TokenType op) {
    switch (op) {
    case TOKEN_PLUS:
        if (left.type == VAL_STRING && right.type == VAL_STRING) {
            char* combined = malloc(strlen(left.value.string) + strlen(right.value.string) + 1);
            strcpy(combined, left.value.string);
            strcat(combined, right.value.string);
            Value v = make_string(combined);
            free(combined);
            return v;
        } else if (left.type == VAL_STRING && right.type == VAL_NUMBER) {
            char buffer[64];
            sprintf(buffer, "%s%g", left.value.string, right.value.number);
            return make_string(buffer);
        } else if (left.type == VAL_NUMBER && right.type == VAL_STRING) {
            char buffer[64];
            sprintf(buffer, "%g%s", left.value.number, right.value.string);
            return make_string(buffer);
        }
        
        return make_number(left.value.number + right.value.number);
    break;

    case TOKEN_MINUS: return make_number(left.value.number - right.value.number);
    case TOKEN_STAR:  return make_number(left.value.number * right.value.number);
    case TOKEN_SLASH: return make_number(left.value.number / right.value.number);
    case TOKEN_CARET: return make_number(pow(left.value.number, right.value.number));
    case TOKEN_LT:    return make_bool(left.value.number < right.value.number);
    case TOKEN_GT:    return make_bool(left.value.number > right.value.number);
    case TOKEN_LE:    return make_bool(left.value.number <= right.value.number);
    case TOKEN_GE:    return make_bool(left.value.number >= right.value.number);
    case TOKEN_EQ:    return make_bool(left.value.number == right.value.number);
    case TOKEN_NE:    return make_bool(left.value.number != right.value.number);
    case TOKEN_AND:   return make_bool(is_true(left) && is_true(right));
    case TOKEN_OR:    return make_bool(is_true(left) || is_true(right));
    
    default:
        printf("Unknown binary operator %d\n", op);
        exit(1);
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

            return (EvalResult){binary_op(left, right, node->Binary.op), NORMAL};
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

        case AST_FOR: {
            Value iterable = (eval(node->For.iterable, scope)).value;
            if (iterable.type != VAL_LIST) {
                printf("For loop error: iterable must be a list\n");
                exit(1);
            }
            EvalResult result = {make_none(), NORMAL};
            List* list = iterable.value.list;
            
            for (int i = 0; i < list->count; i++) {
                scope_set(scope, node->For.var, list->items[i]);

                result = eval(node->For.body, scope);

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

        case AST_LIST: {
            Value v = make_list();
            List* list = v.value.list;
            for (int i = 0; i < node->List.count; i++) {
                Value item = (eval(node->List.elements[i], scope)).value;

                if (list->count >= list->capacity) {
                    list->capacity *= 2;
                    list->items = realloc(list->items, sizeof(Value) * list->capacity);
                }
                list->items[list->count++] = item;
            }

            return (EvalResult){v, NORMAL};
        }

        case AST_INDEX: {
            Value target = (eval(node->Index.target, scope)).value;
            Value index = (eval(node->Index.index, scope)).value;

            if (target.type != VAL_LIST || index.type != VAL_NUMBER) {
                printf("Indexing error: target must be a list and index must be a number\n");
                exit(1);
            }

            int idx = (int)index.value.number;
            if (idx < 0 || idx >= target.value.list->count) {
                printf("Index out of bounds: %d\n", idx);
                exit(1);
            }

            return (EvalResult){target.value.list->items[idx], NORMAL};
        }
        break;

        case AST_ASSIGN_INDEX: {
            Value target = (eval(node->AssignIndex.target, scope)).value;
            Value index = (eval(node->AssignIndex.index, scope)).value;
            Value value = (eval(node->AssignIndex.value, scope)).value;

            if (target.type != VAL_LIST || index.type != VAL_NUMBER) {
                printf("Index assignment error: target must be a list and index must be a number\n");
                exit(1);
            }

            int idx = (int)index.value.number;
            if (idx < 0 || idx >= target.value.list->count) {
                printf("Index out of bounds: %d\n", idx);
                exit(1);
            }

            target.value.list->items[idx] = value;
            return (EvalResult){value, NORMAL};
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
            print_value(val);
            printf("\n");
            return (EvalResult){make_none(), NORMAL};
        }
        break;

        default:
            printf("Unknown AST node type %d\n", node->type);
            exit(1);
    }
}
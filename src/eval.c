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
            // printf ("Scope '%s', Variable: %s = %g\n", scope->name, node->Variable.name, v->value.value.number);
            return v->value;
        }
        break;

        case AST_STRING:
            return make_string(node->String.value);
        break;
        
        case AST_ASSIGN: {
            Value value = eval(node->Assign.value, scope);
            // printf ("Scope '%s', Assign: %s = %g\n", scope->name, node->Assign.name, value.value.number);
            scope_set(scope, node->Assign.name, value);
            return value;
        }
        break;

        case AST_BINARY: {
            Value left = eval(node->Binary.left, scope);
            Value right = eval(node->Binary.right, scope);

            switch (node->Binary.op) {
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
                    printf("Unknown operator\n");
                    exit(1);
            }
        }
        break;

        case AST_UNARY: {
            Value val = eval(node->Unary.value, scope);
            switch (node->Unary.op) {
                case TOKEN_MINUS:
                    return make_number(-val.value.number);
                case TOKEN_NOT:
                    return make_bool(!is_true(val));
                default:
                    printf("Unknown unary operator\n");
                    exit(1);
            }
        }
        break;

        case AST_IF: {
            Value cond = eval(node->If.condition, scope);
            if (is_true(cond)) {
                return eval(node->If.then_branch, scope);
            } else if (node->If.else_branch) {
                return eval(node->If.else_branch, scope);
            }
            return make_none();
        }
        break;

        case AST_WHILE: {
            while (is_true(eval(node->While.condition, scope))) {
                eval(node->While.body, scope);
            }
            return make_none();
        }
        break;

        case AST_BLOCK: {
            Value result = make_none();
            for (int i = 0; i < node->Block.count; i++) {
                result = eval(node->Block.statements[i], scope);
                if (scope->has_return) {
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
            return make_none();
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
            fn_scope.has_return = 0;

            for (int i = 0; i < fn->param_count; i++) {
                Value arg_value = eval(node->Call.args[i], scope);
                scope_set(&fn_scope, fn->params[i], arg_value);
            }

            return eval(fn->body, &fn_scope);
        }
        break;

        case AST_RETURN: {
            Value v = make_none();
            if (node->Return.value) {
                v = eval(node->Return.value, scope);
            }
            scope->has_return = 1;
            scope->return_value = v;
            return v;
        }
        break;

        case AST_PRINT: {
            Value val = eval(node->Print.expr, scope);
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
            return make_none();
        }
        break;

        default:
            printf("Unknown AST node type\n");
            exit(1);
    }
}
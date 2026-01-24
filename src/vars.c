#include "vars.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

Var * scope_find(Scope* scope, const char* name) {
    while (scope) {
        Var* v = scope->vars;
        while (v) {
            if (strcmp(v->name, name) == 0) {
                return v;
            }
            v = v->next;
        }

        scope = scope->parent;
    }

    return NULL;
}

void scope_set(Scope* scope, const char* name, Value value) {
    // Find value in current scope
    Var* v = scope->vars;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            v->value = value;
            return;
        }
        v = v->next;
    }

    // Create new value if not found
    v = malloc(sizeof(Var));
    v->name = strdup(name);
    v->value = value;
    v->next = scope->vars;
    scope->vars = v;
}

int is_true(Value v) {
    switch (v.type) {
        case VAL_BOOL: return v.value.boolean;
        case VAL_FLOAT: return v.value.f != 0;
        case VAL_NONE: return 0;
        default: return 1;
    }
}

Value make_number_int(int x) {
    return (Value){.type = VAL_INT, .value.i = x};
}

Value make_number_float(double x) {
    return (Value){.type = VAL_FLOAT, .value.f = x};
}

Value make_bool(int b) {
    return (Value){.type = VAL_BOOL, .value.boolean = b};
}

Value make_list() {
    List* l = malloc(sizeof(List));
    l->count = 0;
    l->capacity = 4;
    l->items = malloc(sizeof(Value) * l->capacity);
    Value v;
    v.type = VAL_LIST;
    v.value.list = l; // Initialize as needed
    return v;
}

Value make_string(const char* s) {
    Value v;
    v.type = VAL_STRING;
    v.value.string = strdup(s);
    return v;
}

Value make_none() {
    return (Value){.type = VAL_NONE};
}

void print_value(Value v) {
    switch (v.type) {
        case VAL_INT:
            printf("%ld", v.value.i);
        break;
        case VAL_FLOAT:
            printf("%g", v.value.f);
        break;
        case VAL_STRING:
            printf("%s", v.value.string);
        break;
        case VAL_BOOL:
            printf("%s", v.value.boolean ? "True" : "False");
        break;
        case VAL_NONE:
            printf("None");
        break;
        case VAL_FUNCTION:
            printf("<function>");
        break;
        case VAL_LIST: {
            printf("[");
            for (int i = 0; i < v.value.list->count; i++) {
                print_value(v.value.list->items[i]);
                if (i < v.value.list->count - 1) {
                    printf(", ");
                }
            }
            printf("]");
        }
        break;
        default:
            printf("<unknown>");
    }
}

void free_var(Var* v) {
    if (!v) return;

    if (v->value.type == VAL_STRING && v->value.value.string) {
        free(v->value.value.string);
    }

    if (v->value.type == VAL_FUNCTION && v->value.value.function) {
        Function* fn = v->value.value.function;
        free(fn->name);
        for (int i = 0; i < fn->param_count; i++) {
            free(fn->params[i]);
        }
        free(fn->params);
        free(fn);
    }

    free(v);
}

void free_scope(Scope* scope) {
    if (!scope) return;

    Var* v = scope->vars;
    while (v) {
        Var* next = v->next;
        free_var(v);
        v = next;
    }

    scope->vars = NULL;
    free(scope);
}

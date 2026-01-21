#include "vars.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

static Var* var_list = NULL;

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

Value make_number(double x) {
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
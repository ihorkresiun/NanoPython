#include "vars.h"

#include "string.h"
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
    return (Value){.type = VAL_NUMBER, .value.number = x};
}

Value make_bool(int b) {
    return (Value){.type = VAL_BOOL, .value.boolean = b};
}

Value make_none() {
    return (Value){.type = VAL_NONE};
}
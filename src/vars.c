#include "vars.h"

#include "string.h"
#include "stdlib.h"

static Var* var_list = NULL;

/*
Var* var_find(const char* name) {
    Var* v = var_list;
    while (v) {
        if (strcmp(v->name, name) == 0)
            return v;
        v = v->next;
    }
    return NULL;
}
*/

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
    // Change current value if exist
    Var* v = scope_find(scope, name);
    if (v) {
        v->value = value;
        return;
    }

    // Create new value
    v = malloc(sizeof(Var));
    v->name = strdup(name);
    v->value = value;
    v->next = scope->vars;
    scope->vars = v;
}

/*
void var_set(const char* name, double value) {
    Var* v = var_find(name);
    if (v) {
        v->value = value;
        return;
    }

    v = malloc(sizeof(Var));
    v->name = strdup(name);
    v->value = value;
    v->next = var_list;
    var_list = v;
}
*/

Value make_number(double x) {
    return (Value){.type = VAL_NUMBER, .value.number = x};
}

Value make_bool(int b) {
    return (Value){.type = VAL_BOOL, .value.boolean = b};
}

Value make_none() {
    return (Value){.type = VAL_NONE};
}
#include "vars.h"

#include "string.h"
#include "stdlib.h"

static Var* var_list = NULL;

Var* var_find(const char* name) {
    Var* v = var_list;
    while (v) {
        if (strcmp(v->name, name) == 0)
            return v;
        v = v->next;
    }
    return NULL;
}

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
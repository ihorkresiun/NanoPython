#ifndef __INC_VARS_H__
#define __INC_VARS_H__

#include "ast.h"

typedef struct Var {
    const char* name;
    Value value;
    struct Var* next;
} Var;

typedef struct Scope {
    const char * name;
    Var * vars;
    Value return_value;
    struct Scope* parent;
}Scope;

Var * scope_find(Scope* scope, const char* name);
void scope_set(Scope* scope, const char* name, Value value);

Value make_number(double x);
Value make_bool(int b);
Value make_string(const char* s);
Value make_none();

#endif // __INC_VARS_H__
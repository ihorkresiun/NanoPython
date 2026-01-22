#ifndef __INC_VARS_H__
#define __INC_VARS_H__

#include "ast.h"

typedef struct Ast Ast; // forward declaration
typedef struct Scope Scope; // forward declaration

typedef struct Function {
    char* name;
    char** params;
    int param_count;
    Ast* body;
    Scope* scope; // Closure scope
}Function;

typedef enum {
    VAL_NONE,
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_LIST,
    VAL_FUNCTION,
    VAL_STRING,
}ValueType;

typedef struct List List; // forward declaration

typedef struct Value {
    ValueType type;
    union {
        long i;
        double f;
        int boolean;
        char* string;
        List* list;
        struct Function* function;
    }value;
} Value;

typedef struct List {
    int count;
    int capacity;
    Value* items;
} List;

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

int is_true(Value v);

Value make_number(double x);
Value make_bool(int b);
Value make_list();
Value make_string(const char* s);
Value make_none();

void print_value(Value v);

#endif // __INC_VARS_H__
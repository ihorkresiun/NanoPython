#ifndef __INC_VARS_H__
#define __INC_VARS_H__

typedef enum {
    VAL_NUMBER,
    VAL_BOOL,
    VAL_NONE,
    VAL_FUNCTION
}ValueType;

typedef struct Value {
    ValueType type;
    union {
        double number;
        int boolean;
        // struct Function* function;
        void * any;
    }value;
} Value;

typedef struct Var {
    char* name;
    Value value;
    struct Var* next;
} Var;

typedef struct Scope {
    Var * vars;
    struct Scope* parent;
}Scope;

typedef struct Function {
    char** params;
    int param_count
    Ast* body;
    Scope* closure;
}Function;
/*
Var* var_find(const char* name);
void var_set(const char* name, double value);
*/

Var * scope_find(Scope* scope, const char* name);
void scope_set(Scope* scope, const char* name, Value value);

Value make_number(double x);
Value make_bool(int b);
Value make_none();

#endif // __INC_VARS_H__
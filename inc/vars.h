#ifndef __INC_VARS_H__
#define __INC_VARS_H__

#include "ast.h"

typedef struct Ast Ast; // forward declaration
typedef struct Scope Scope; // forward declaration

typedef enum {
    VAL_NONE,
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_OBJ,
}ValueType;

typedef enum {
    OBJ_STRING,
    OBJ_LIST,
    OBJ_DICT,
    OBJ_FUNCTION,
    OBJ_NATIVE_FUNCTION,
}ObjectType;

typedef struct Obj {
    ObjectType type;
    struct Obj* next;
}Obj;

typedef struct Value {
    ValueType type;
    union {
        long integer;
        double floating;
        int boolean;
        Obj* object;
    }as;
} Value;

typedef struct ObjString{
    Obj obj;
    int length;
    char* chars;
} ObjString;

typedef struct ObjList {
    Obj obj;
    int count;
    int capacity;
    Value* items;
} ObjList;

typedef struct ObjDict {
    Obj obj;
    int count;
    int capacity;
    char** keys;
    Value* values;
} ObjDict;

typedef struct ObjFunction {
    Obj obj;
    int addr; // Address in bytecode
    char* name;
    char** params;
    int param_count;
    // Ast* body;
    Scope* scope; // Closure scope
} ObjFunction;

typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct ObjNativeFunction {
    Obj obj;
    NativeFn function;
    char* name;
} ObjNativeFunction;

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
int is_obj_type(Value v, ObjectType type);
ObjString* as_string(Value v);

Value make_number_int(int x);
Value make_number_float(double x);
Value make_bool(int b);

Value make_string(const char* s);
Value make_list();
Value make_dict();
Value make_function(ObjFunction* fn);
Value make_native_function(const char* name, NativeFn function);

Value make_none();

void print_value(Value v);

int value_equals(Value* a, Value* b);

void free_var(Var* v);
void free_scope(Scope* scope);

#endif // __INC_VARS_H__
#ifndef __INC_VARS_H__
#define __INC_VARS_H__

typedef struct Var {
    char* name;
    double value;
    struct Var* next;
} Var;


Var* var_find(const char* name);
void var_set(const char* name, double value);

#endif // __INC_VARS_H__
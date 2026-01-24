#ifndef INC_COMPILER_H
#define INC_COMPILER_H

#include "ast.h"
#include "vm.h"

typedef struct {
    int loop_start;
    int* break_jumps;
    int break_count;
    int break_capacity;
} LoopContext;

#define MAX_LOOP_NESTING 16

typedef struct {
    Bytecode* bytecode;

    LoopContext loop_stack[MAX_LOOP_NESTING];
    int loop_count;
} Compiler;

void compile(Compiler* compiler, Ast* node);

#endif // INC_COMPILER_H

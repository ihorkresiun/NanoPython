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

typedef struct {
    Bytecode* bytecode;

    LoopContext loop_stack[16];
    int loop_count;
} Compiler;

void compile(Compiler* compiler, Ast* node);

#endif // INC_COMPILER_H

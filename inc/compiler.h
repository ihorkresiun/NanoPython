#ifndef INC_COMPILER_H
#define INC_COMPILER_H

#include "ast.h"
#include "vm.h"
#include "hashmap.h"

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
    HashMap imported_modules;  // Track imported modules to avoid duplicates
    HashMap string_constants; // Map string values to their constant pool indices
} Compiler;

void compiler_init(Compiler* compiler);
Bytecode* compile(Compiler* compiler, Ast* node);
void compiler_free(Compiler* compiler);

#endif // INC_COMPILER_H

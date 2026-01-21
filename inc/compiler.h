#ifndef INC_COMPILER_H
#define INC_COMPILER_H

#include "ast.h"
#include "vm.h"

typedef struct {
    Bytecode* bytecode;
} Compiler;

void compile(Compiler* compiler, Ast* node);

#endif // INC_COMPILER_H

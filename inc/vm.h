#ifndef __INC_VM_H__
#define __INC_VM_H__

#include "vars.h"

typedef enum {
    OP_NOP,

    OP_LOAD,
    OP_STORE,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_EQ,
    OP_LT,
    OP_GT,

    OP_JUMP,
    OP_JUMP_IF_ZERO,

    OP_CONST,
    OP_POP,

    OP_PRINT,

    OP_STORE_GLOBAL,
    OP_LOAD_GLOBAL,

    OP_CALL,
    OP_RET,

    OP_HALT
} Opcode;

typedef struct {
    Opcode opcode;
    int operand;
} Instruction;

typedef struct {
    Instruction* instructions;
    int count;
    int capacity;

    Value* constants;
    int const_count;
} Bytecode;


typedef struct CallFrame {
    int return_address;
    int base_sp;
    Scope* scope;
} CallFrame;

#define VM_STACK_SIZE 1024
#define MAX_CALL_STACK_SIZE 64

typedef struct {
    Bytecode* bytecode;
    Value stack[VM_STACK_SIZE];
    int sp; // Stack pointer
    int ip; // Instruction pointer

    CallFrame call_stack[MAX_CALL_STACK_SIZE];
    int frame_count;
    Scope* scope;
} VM;

void vm_run(VM* vm);

#endif /* __INC_VM_H__ */

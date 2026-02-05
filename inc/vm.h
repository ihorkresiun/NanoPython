#ifndef __INC_VM_H__
#define __INC_VM_H__

#include "gc.h"
#include "vars.h"
#include "hashmap.h"

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

    OP_CALL,
    OP_RET,

    OP_MAKE_LIST,
    OP_MAKE_TUPLE,
    OP_MAKE_SET,

    OP_IDX_GET,
    OP_IDX_SET,

    OP_MAKE_CLASS,
    OP_MAKE_INSTANCE,
    OP_GET_ATTR,
    OP_SET_ATTR,
    OP_CALL_METHOD,

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

typedef struct VM{
    Bytecode* bytecode;
    Value stack[VM_STACK_SIZE];
    int sp; // Stack pointer
    int ip; // Instruction pointer

    CallFrame call_stack[MAX_CALL_STACK_SIZE];
    int frame_count;
    Scope* scope;
    HashMap strings; // For string interning

    Obj* objects; // Linked list of all allocated objects for GC
    int bytes_allocated;
    int next_gc; // Threshold to trigger next GC
} VM;

void vm_run(VM* vm);
void vm_init(VM* vm, Bytecode* bytecode);

void vm_push(VM* vm, Value value);
Value vm_pop(VM* vm);

void vm_register_native_functions(VM* vm, const char* name, NativeFn function);

#endif /* __INC_VM_H__ */

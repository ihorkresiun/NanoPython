#include "vm.h"

#include "stdio.h"
#include "stdlib.h"

static void vm_init(VM* vm, Bytecode* bytecode) {
    vm->bytecode = bytecode;
    vm->sp = 0;
    vm->ip = 0;
}

static void vm_push(VM* vm, Value value) {
    if (vm->sp >= VM_STACK_SIZE - 1) {
        printf("Stack overflow\n");
        exit(1);
    }
    vm->stack[vm->sp++] = value;
}

static Value vm_pop(VM* vm) {
    if (vm->sp <= 0) {
        printf("Stack underflow\n");
        exit(1);
    }
    return vm->stack[--vm->sp];
}

static Value vm_peek(VM* vm) {
    return vm->stack[vm->sp - 1];
}

void vm_run(VM* vm) {
    while (1) {
        Instruction instr = vm->bytecode->instructions[vm->ip++];
        switch (instr.opcode) {
            case OP_CONST: {
                Value constant = vm->bytecode->constants[instr.operand];
                vm_push(vm, constant);
            }
            break;

            case OP_POP: {
                vm_pop(vm);
            }
            break;

            case OP_ADD: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result = make_number(a.value.f + b.value.f);
                vm_push(vm, result);
            }
            break;

            case OP_SUB: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result = make_number(a.value.f - b.value.f);
                vm_push(vm, result);
            }
            break;

            case OP_LOAD: {
                Value v = vm->bytecode->constants[instr.operand];
                vm_push(vm, v);
            }
            break;

            case OP_STORE: {
                Value v = vm_pop(vm);
                scope_set(vm->globals, "Global", v);
            }
            break;

            case OP_PRINT: {
                Value v = vm_pop(vm);
                print_value(v);
                printf("\n");
            }
            break;

            case OP_NOP:
                // Do nothing
            break;

            case OP_RET:
                return;

            case OP_HALT:
                return;

            default:
                printf("Unknown opcode %d\n", instr.opcode);
                exit(1);
        }
    }
}
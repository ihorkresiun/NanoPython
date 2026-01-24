#include "vm.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void vm_init(VM* vm, Bytecode* bytecode) {
    vm->bytecode = bytecode;
    vm->sp = 0;
    vm->ip = 0;
    vm->frame_count = 0;
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
                Value result;
                if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    result = make_number_float(a.value.f + b.value.f);
                } else {
                    result = make_number_int(a.value.i + b.value.i);
                }
                vm_push(vm, result);
            }
            break;

            case OP_SUB: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result;
                if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    result = make_number_float(a.value.f - b.value.f);
                } else {
                    result = make_number_int(a.value.i - b.value.i);
                }
                vm_push(vm, result);
            }
            break;

            case OP_MUL: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result;
                if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    result = make_number_float(a.value.f * b.value.f);
                } else {
                    result = make_number_int(a.value.i * b.value.i);
                }
                vm_push(vm, result);
            }
            break;

            case OP_DIV: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result;
                if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    result = make_number_float(a.value.f / b.value.f);
                } else {
                    result = make_number_int(a.value.i / b.value.i);
                }
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
                scope_set(vm->scope, "Global", v);
            }
            break;

            case OP_PRINT: {
                Value v = vm_pop(vm);
                print_value(v);
                printf("\n");
            }
            break;

            case OP_STORE_GLOBAL: {
                Value v = vm_pop(vm);
                Value name_val = vm->bytecode->constants[instr.operand];
                if (name_val.type != VAL_STRING) {
                    printf("STORE_GLOBAL expects a string constant as variable name\n");
                    exit(1);
                }
                scope_set(vm->scope, name_val.value.string, v);
            }
            break;

            case OP_LOAD_GLOBAL: {
                Value name_val = vm->bytecode->constants[instr.operand];
                if (name_val.type != VAL_STRING) {
                    printf("LOAD_GLOBAL expects a string constant as variable name\n");
                    exit(1);
                }
                Var* var = scope_find(vm->scope, name_val.value.string);
                if (!var) {
                    printf("Undefined global variable: %s\n", name_val.value.string);
                    exit(1);
                }
                vm_push(vm, var->value);
            }
            break;

            case OP_JUMP_IF_ZERO: {
                Value cond = vm_pop(vm);
                if (!is_true(cond)) {
                    vm->ip = instr.operand;
                }
            }
            break;

            case OP_EQ: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result = make_bool(a.value.f == b.value.f);
                vm_push(vm, result);
            }
            break;

            case OP_LT: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result = make_bool(a.value.f < b.value.f);
                vm_push(vm, result);
            }
            break;

            case OP_GT: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                Value result = make_bool(a.value.f > b.value.f);
                vm_push(vm, result);
            }
            break;
            
            case OP_JUMP: {
                vm->ip = instr.operand;
            }
            break;

            case OP_NOP:
                // Do nothing
            break;

            case OP_CALL: {
                Value func_val = vm_pop(vm);
                if (func_val.type != VAL_FUNCTION) {
                    printf("Attempted to call a non-function value\n");
                    exit(1);
                }
                if (vm->frame_count >= MAX_CALL_STACK_SIZE) {
                    printf("Call stack overflow\n");
                    exit(1);
                }

                Function* fn = func_val.value.function;
                Scope *new_scope = malloc(sizeof(Scope));
                new_scope->name = fn->name;
                new_scope->parent = fn->scope ? fn->scope : vm->scope;

                for (int i = fn->param_count - 1; i >= 0; i--) {
                    Value arg_val = vm_pop(vm);
                    Var* var = malloc(sizeof(Var));
                    var->name = strdup(fn->params[i]);
                    var->value = arg_val;
                    var->next = new_scope->vars;
                    new_scope->vars = var;
                }

                CallFrame* frame = &vm->call_stack[vm->frame_count++];
                frame->return_address = vm->ip;
                frame->scope = vm->scope;
                frame->base_sp = vm->sp;

                vm->scope = new_scope;
                vm->ip = fn->addr;
            }
            break;

            case OP_RET: {
                if (vm->frame_count <= 0) {
                    printf("Call stack underflow\n");
                    exit(1);
                }
                CallFrame* frame = &vm->call_stack[--vm->frame_count];
                // Restore previous frame state
                vm->scope = frame->scope;
                Value ret_val = vm_pop(vm);
                vm->sp = frame->base_sp;
                vm->ip = frame->return_address;
                vm_push(vm, ret_val);
                // TODO: Free the function scope
                //free_scope(frame->scope);
                
            }
            break;

            case OP_HALT:
                return;

            default:
                printf("VM: Unknown opcode %d\n", instr.opcode);
                exit(1);
        }
    }
}
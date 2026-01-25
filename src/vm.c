#include "vm.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void vm_init(VM* vm, Bytecode* bytecode);
static void vm_push(VM* vm, Value value);
static Value vm_pop(VM* vm);
static Value vm_peek(VM* vm);
static void op_const_to_stack(VM* vm, int operand);
static void op_add(VM* vm);
static void op_sub(VM* vm);
static void op_mul(VM* vm);
static void op_div(VM* vm);
static void op_print(VM* vm);
static void op_store_global(VM* vm, int operand);
static void op_load_global(VM* vm, int operand);
static void op_jump_if_zero(VM* vm, int operand);
static void op_equal(VM* vm);
static void op_less_than(VM* vm);
static void op_greater_than(VM* vm);
static void op_call(VM* vm);
static void op_return(VM* vm);
static void op_make_list(VM* vm, int operand);
static void op_list_get(VM* vm);
static void op_list_set(VM* vm);

void vm_run(VM* vm) 
{
    while (1) {
        Instruction instr = vm->bytecode->instructions[vm->ip++];
        switch (instr.opcode) {
            case OP_CONST: op_const_to_stack(vm, instr.operand); break;
            case OP_POP: vm_pop(vm); break;
            case OP_ADD: op_add(vm); break;
            case OP_SUB: op_sub(vm); break;
            case OP_MUL: op_mul(vm); break;
            case OP_DIV: op_div(vm); break;
            case OP_PRINT: op_print(vm); break;
            case OP_STORE: op_store_global(vm, instr.operand); break;
            case OP_LOAD: op_load_global(vm, instr.operand); break;
            case OP_JUMP_IF_ZERO: op_jump_if_zero(vm, instr.operand); break;
            case OP_EQ: op_equal(vm); break;
            case OP_LT: op_less_than(vm); break;
            case OP_GT: op_greater_than(vm); break;
            case OP_JUMP: vm->ip = instr.operand; break;
            case OP_NOP: break;
            case OP_CALL: op_call(vm); break;
            case OP_RET: op_return(vm); break;
            case OP_MAKE_LIST: op_make_list(vm, instr.operand); break;
            case OP_LIST_GET: op_list_get(vm); break;
            case OP_LIST_SET: op_list_set(vm); break;
            case OP_HALT: return;

            default:
                printf("VM: Unknown opcode %d\n", instr.opcode);
                exit(1);
        }
    }
}

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

static void op_const_to_stack(VM* vm, int operand) {
    Value constant = vm->bytecode->constants[operand];
    vm_push(vm, constant);
}

static void op_add(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result;
    if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result = make_number_float(a.value.f + b.value.f);
    } else if (a.type == VAL_FLOAT && b.type == VAL_INT) {
        result = make_number_float(a.value.f + b.value.i);
    } else if (a.type == VAL_INT && b.type == VAL_FLOAT) {
        result = make_number_float(a.value.i + b.value.f);
    } else if (a.type == VAL_INT && b.type == VAL_INT) {
        result = make_number_int(a.value.i + b.value.i);
    } else if (a.type == VAL_STRING && b.type == VAL_STRING) {
        int len_a = strlen(a.value.string);
        int len_b = strlen(b.value.string);
        char* concatenated = malloc(len_a + len_b + 1);
        strcpy(concatenated, a.value.string);
        strcat(concatenated, b.value.string);
        result = (Value){.type = VAL_STRING, .value.string = concatenated};
    } else if (a.type == VAL_STRING && b.type == VAL_INT) {
        int len_a = strlen(a.value.string);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%ld", b.value.i);
        int len_b = strlen(buffer);
        char* concatenated = malloc(len_a + len_b + 1);
        strcpy(concatenated, a.value.string);
        strcat(concatenated, buffer);
        result = (Value){.type = VAL_STRING, .value.string = concatenated};
    } else if (a.type == VAL_INT && b.type == VAL_STRING) {
        int len_b = strlen(b.value.string);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%ld", a.value.i);
        int len_a = strlen(buffer);
        char* concatenated = malloc(len_a + len_b + 1);
        strcpy(concatenated, buffer);
        strcat(concatenated, b.value.string);
        result = (Value){.type = VAL_STRING, .value.string = concatenated};
    } else if (a.type == VAL_STRING && b.type == VAL_FLOAT) {
        int len_a = strlen(a.value.string);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%f", b.value.f);
        int len_b = strlen(buffer);
        char* concatenated = malloc(len_a + len_b + 1);
        strcpy(concatenated, a.value.string);
        strcat(concatenated, buffer);
        result = (Value){.type = VAL_STRING, .value.string = concatenated};
    } else if (a.type == VAL_FLOAT && b.type == VAL_STRING) {
        int len_b = strlen(b.value.string);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%f", a.value.f);
        int len_a = strlen(buffer);
        char* concatenated = malloc(len_a + len_b + 1);
        strcpy(concatenated, buffer);
        strcat(concatenated, b.value.string);
        result = (Value){.type = VAL_STRING, .value.string = concatenated};
    } 
    
    else {
        printf("Unsupported types for ADD operation\n");
        exit(1);
    }
    vm_push(vm, result);
}

static void op_sub(VM* vm) {
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

static void op_mul(VM* vm) {
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

static void op_div(VM* vm) {
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

static void op_print(VM* vm) {
    Value v = vm_pop(vm);
    print_value(v);
    printf("\n");
}

static void op_store_global(VM* vm, int operand) {
    Value v = vm_pop(vm);
    Value name_val = vm->bytecode->constants[operand];
    if (name_val.type != VAL_STRING) {
        printf("STORE_GLOBAL expects a string constant as variable name\n");
        exit(1);
    }
    scope_set(vm->scope, name_val.value.string, v);
}

static void op_load_global(VM* vm, int operand) {
    Value name_val = vm->bytecode->constants[operand];
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

static void op_jump_if_zero(VM* vm, int operand) {
    Value cond = vm_pop(vm);
    if (!is_true(cond)) {
        vm->ip = operand;
    }
}

static void op_equal(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result = make_bool(a.value.f == b.value.f);
    vm_push(vm, result);
}

static void op_less_than(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result = make_bool(a.value.f < b.value.f);
    vm_push(vm, result);
}

static void op_greater_than(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result = make_bool(a.value.f > b.value.f);
    vm_push(vm, result);
}

static void op_call(VM* vm) 
{
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
    new_scope->vars = NULL;

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


void op_return(VM* vm) {
    if (vm->frame_count <= 0) {
        printf("Call stack underflow\n");
        exit(1);
    }
    CallFrame* frame = &vm->call_stack[--vm->frame_count];
    // Free current scope
    free_scope(vm->scope);
    // Restore previous frame state
    vm->scope = frame->scope;
    Value ret_val = vm_pop(vm);
    vm->sp = frame->base_sp;
    vm->ip = frame->return_address;
    vm_push(vm, ret_val);
}

void op_make_list(VM* vm, int count) {
    List* list = malloc(sizeof(List));
    list->count = count;
    list->capacity = count;
    list->items = malloc(sizeof(Value) * count);
    for (int i = count - 1; i >= 0; i--) {
        list->items[i] = vm_pop(vm);
    }
    Value list_val = {.type = VAL_LIST, .value.list = list};
    vm_push(vm, list_val);
}

void op_list_get(VM* vm) {
    Value index_val = vm_pop(vm);
    Value list_val = vm_pop(vm);
    if (list_val.type != VAL_LIST) {
        printf("LIST_GET expects a list value\n");
        exit(1);
    }

    List* list = list_val.value.list;
    int index = (int)index_val.value.i;
    if (index < 0 || index >= list->count) {
        printf("LIST_GET index out of bounds\n");
        exit(1);
    }
    Value item = list->items[index];
    vm_push(vm, item);
}

void op_list_set(VM* vm) {
    Value value = vm_pop(vm);
    Value index_val = vm_pop(vm);
    Value list_val = vm_pop(vm);
    if (list_val.type != VAL_LIST) {
        printf("LIST_SET expects a list value\n");
        exit(1);
    }

    List* list = list_val.value.list;
    int index = (int)index_val.value.i;
    if (index < 0 || index >= list->count) {
        printf("LIST_SET index out of bounds\n");
        exit(1);
    }
    list->items[index] = value;
}

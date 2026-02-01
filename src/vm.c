#include "vm.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void vm_push(VM* vm, Value value);
static Value vm_pop(VM* vm);
static Value vm_peek(VM* vm);
static void op_const_to_stack(VM* vm, int operand);
static void op_add(VM* vm);
static void op_sub(VM* vm);
static void op_mul(VM* vm);
static void op_div(VM* vm);
static void op_store_global(VM* vm, int operand);
static void op_load_global(VM* vm, int operand);
static void op_jump_if_zero(VM* vm, int operand);
static void op_equal(VM* vm);
static void op_less_than(VM* vm);
static void op_greater_than(VM* vm);
static void op_call(VM* vm, int operand);
static void op_return(VM* vm);
static void op_make_list(VM* vm, int operand);
static void op_make_dict(VM* vm, int count);
static void op_index_get(VM* vm);
static void op_index_set(VM* vm);

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
            case OP_STORE: op_store_global(vm, instr.operand); break;
            case OP_LOAD: op_load_global(vm, instr.operand); break;
            case OP_JUMP_IF_ZERO: op_jump_if_zero(vm, instr.operand); break;
            case OP_EQ: op_equal(vm); break;
            case OP_LT: op_less_than(vm); break;
            case OP_GT: op_greater_than(vm); break;
            case OP_JUMP: vm->ip = instr.operand; break;
            case OP_NOP: break;
            case OP_CALL: op_call(vm, instr.operand); break;
            case OP_RET: op_return(vm); break;
            case OP_MAKE_LIST: op_make_list(vm, instr.operand); break;
            case OP_MAKE_DICT: op_make_dict(vm, instr.operand); break;
            case OP_IDX_GET: op_index_get(vm); break;
            case OP_IDX_SET: op_index_set(vm); break;
            case OP_HALT: return;

            default:
                printf("VM: Unknown opcode %d\n", instr.opcode);
                exit(1);
        }
    }
}

void vm_init(VM* vm, Bytecode* bytecode) {
    vm->bytecode = bytecode;
    vm->sp = 0;
    vm->ip = 0;
    vm->frame_count = 0;
    Scope* global_scope = malloc(sizeof(Scope));
    global_scope->name = "Global";
    global_scope->vars = NULL;
    global_scope->parent = NULL;
    global_scope->return_value = make_none();
    vm->scope = global_scope;
}

void vm_register_native_functions(VM* vm, const char* name, NativeFn function) {
    Value native_fn_val = make_native_function(name, function);
    scope_set(vm->scope, name, native_fn_val);
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
        result = make_number_float(a.as.floating + b.as.floating);
    } else if (a.type == VAL_FLOAT && b.type == VAL_INT) {
        result = make_number_float(a.as.floating + b.as.integer);
    } else if (a.type == VAL_INT && b.type == VAL_FLOAT) {
        result = make_number_float(a.as.integer + b.as.floating);
    } else if (a.type == VAL_INT && b.type == VAL_INT) {
        result = make_number_int(a.as.integer + b.as.integer);
    } else {
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
        result = make_number_float(a.as.floating - b.as.floating);
    } else {
        result = make_number_int(a.as.integer - b.as.integer);
    }
    vm_push(vm, result);
}

static void op_mul(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result;
    if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
        result = make_number_float(a.as.floating * b.as.floating);
    } else {
        result = make_number_int(a.as.integer * b.as.integer);
    }
    vm_push(vm, result);
}

static void op_div(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result;
    if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
        result = make_number_float(a.as.floating / b.as.floating);
    } else {
        result = make_number_int(a.as.integer / b.as.integer);
    }
    vm_push(vm, result);
}

static void op_store_global(VM* vm, int operand) {
    Value v = vm_pop(vm);
    Value name_val = vm->bytecode->constants[operand];
    if (!is_obj_type(name_val, OBJ_STRING)) {
        printf("STORE_GLOBAL expects a string constant as variable name\n");
        exit(1);
    }
    scope_set(vm->scope, as_string(name_val)->chars, v);
}

static void op_load_global(VM* vm, int operand) {
    Value name_val = vm->bytecode->constants[operand];
    if (!is_obj_type(name_val, OBJ_STRING)) {
        printf("LOAD_GLOBAL expects a string constant as variable name\n");
        exit(1);
    }
    Var* var = scope_find(vm->scope, as_string(name_val)->chars);
    if (!var) {
        printf("Undefined global variable: %s\n", as_string(name_val)->chars);
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
    Value result = make_bool(a.as.floating == b.as.floating);
    vm_push(vm, result);
}

static void op_less_than(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result = make_bool(a.as.floating < b.as.floating);
    vm_push(vm, result);
}

static void op_greater_than(VM* vm) {
    Value b = vm_pop(vm);
    Value a = vm_pop(vm);
    Value result = make_bool(a.as.floating > b.as.floating);
    vm_push(vm, result);
}

static void op_call(VM* vm, int operand) 
{
    Value func_val = vm_pop(vm);
    if (func_val.type != VAL_OBJ) {
        printf("Attempted to call a non-function value\n");
        exit(1);
    }
    if (vm->frame_count >= MAX_CALL_STACK_SIZE) {
        printf("Call stack overflow\n");
        exit(1);
    }

    if (func_val.as.object->type == OBJ_NATIVE_FUNCTION) {
        ObjNativeFunction* native_fn = (ObjNativeFunction*)func_val.as.object;
        // For simplicity, assume no arguments for native functions
        int arg_count = operand;
        Value result = native_fn->function(arg_count, &vm->stack[vm->sp - arg_count]);
        vm_push(vm, result);
        return;
    }

    ObjFunction* fn = (ObjFunction*)func_val.as.object;
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
    ObjList* list = malloc(sizeof(ObjList));
    list->count = count;
    list->capacity = count;
    list->items = malloc(sizeof(Value) * count);
    for (int i = count - 1; i >= 0; i--) {
        list->items[i] = vm_pop(vm);
    }
    Value list_val;
    list_val.type = VAL_OBJ;
    list_val.as.object = (Obj*)list;
    list_val.as.object->type = OBJ_LIST;
    vm_push(vm, list_val);
}

static void op_make_dict(VM* vm, int count) {
    ObjDict* dict = malloc(sizeof(ObjDict));
    dict->count = count;
    dict->capacity = count;
    dict->keys = malloc(sizeof(char*) * count);
    dict->values = malloc(sizeof(Value) * count);

    for (int i = count - 1; i >= 0; i--) {
        Value val = vm_pop(vm);
        Value key = vm_pop(vm);

        if (!is_obj_type(key, OBJ_STRING)) {
            printf("Dictionary keys must be strings\n");
            exit(1);
        }
        dict->keys[i] = strdup(as_string(key)->chars);
        dict->values[i] = val;
    }

    Value dict_val;
    dict_val.type = VAL_OBJ;
    dict_val.as.object = (Obj*)dict;
    dict_val.as.object->type = OBJ_DICT;
    vm_push(vm, dict_val);
}

static int find_by_key(const ObjDict* dict, const char* key) {
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) {
            return i;
        }
    }
    return -1;
}

static void op_index_get(VM* vm) {
    Value index_val = vm_pop(vm);
    Value list_val = vm_pop(vm);
    if (is_obj_type(list_val, OBJ_LIST)) {

        ObjList* list = (ObjList*)list_val.as.object;
        int index = (int)index_val.as.integer;
        if (index < 0 || index >= list->count) {
            printf("LIST_GET index out of bounds\n");
            exit(1);
        }
        Value item = list->items[index];
        vm_push(vm, item);
    }

    if (is_obj_type(list_val, OBJ_DICT)) {
        ObjDict* dict = (ObjDict*)list_val.as.object;
        if (!is_obj_type(index_val, OBJ_STRING)) {
            printf("DICT_GET expects a string key\n");
            exit(1);
        }
        const char* key = as_string(index_val)->chars;
        int idx = find_by_key(dict, key);
        if (idx != -1) {
            vm_push(vm, dict->values[idx]);
            return;
        }
        printf("Key not found in dictionary: %s\n", key);
        exit(1);
    }

    printf("IDX_GET expects a list or dictionary value\n");
    exit(1);
    
}

static void op_index_set(VM* vm) {
    Value value = vm_pop(vm);
    Value index_val = vm_pop(vm);
    Value list_val = vm_pop(vm);
    if (is_obj_type(list_val, OBJ_LIST)) {

        ObjList* list = (ObjList*)list_val.as.object;
        int index = (int)index_val.as.integer;
        if (index < 0 || index >= list->count) {
            printf("LIST_SET index out of bounds\n");
            exit(1);
        }
        list->items[index] = value;
    }

    if (is_obj_type(list_val, OBJ_DICT)) {
        ObjDict* dict = (ObjDict*)list_val.as.object;
        if (!is_obj_type(index_val, OBJ_STRING)) {
            printf("DICT_SET expects a string key\n");
            exit(1);
        }

        const char* key = as_string(index_val)->chars;
        int idx = find_by_key(dict, key);
        if (idx != -1) {
            dict->values[idx] = value;
            return;
        }

        // Key not found, add new key-value pair
        if (dict->count >= dict->capacity) {
            dict->capacity *= 2;
            dict->keys = realloc(dict->keys, sizeof(char*) * dict->capacity);
            dict->values = realloc(dict->values, sizeof(Value) * dict->capacity);
        }
        dict->keys[dict->count] = strdup(key);
        dict->values[dict->count] = value;
        dict->count++;
        return;
    }
    printf("IDX_SET expects a list or dictionary value\n");
    exit(1);
}

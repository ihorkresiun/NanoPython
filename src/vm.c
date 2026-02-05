#include "vm.h"

#include "hashmap.h"
#include "intern_string.h"
#include "vars.h"
#include "vm_objects.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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
static void op_index_get(VM* vm);
static void op_index_set(VM* vm);
static void op_make_class(VM* vm, int operand);
static void op_make_instance(VM* vm);
static void op_get_attr(VM* vm, int operand);
static void op_set_attr(VM* vm, int operand);
static void op_call_method(VM* vm, int operand);

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
            case OP_IDX_GET: op_index_get(vm); break;
            case OP_IDX_SET: op_index_set(vm); break;
            case OP_MAKE_CLASS: op_make_class(vm, instr.operand); break;
            case OP_MAKE_INSTANCE: op_make_instance(vm); break;
            case OP_GET_ATTR: op_get_attr(vm, instr.operand); break;
            case OP_SET_ATTR: op_set_attr(vm, instr.operand); break;
            case OP_CALL_METHOD: op_call_method(vm, instr.operand); break;
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
    Scope* global_scope = new_scope("Global", NULL);
    vm->scope = global_scope;
    hash_init(&vm->strings, 1024);

    vm->objects = NULL;
    vm->bytes_allocated = 0;
    vm->next_gc = 1024 * 8; // 8KB initial threshold
}

void vm_register_native_functions(VM* vm, const char* name, NativeFn function) {
    Value native_fn_val = make_native_function(name, function);
    ObjString* name_str = intern_const_string(vm, name, strlen(name));
    scope_set(vm->scope, name_str, native_fn_val);
}

void vm_debug_scope(VM* vm) {
    printf("Current Scope Variables:\n");
    Scope* scope = vm->scope;
    while (scope) {
        printf("Scope: %s\n", scope->name);
        for (int i = 0; i < scope->vars->capacity; i++) {
            HashNode* node = &scope->vars->nodes[i];
            while (node != NULL) {
                if (node->key != NULL) {
                    printf("  %s: ", node->key->chars);
                    print_value(node->value);
                    printf("\n");
                }
                node = node->next;
            }
        }
        scope = scope->parent;
    }
}

void vm_debug_stack(VM* vm) {
    printf("VM Stack (sp=%d):\n", vm->sp);
    for (int i = 0; i < vm->sp; i++) {
        printf("  [%d]: ", i);
        print_value(vm->stack[i]);
        printf("\n");
    }
}

void vm_push(VM* vm, Value value) {
    if (vm->sp >= VM_STACK_SIZE - 1) {
        printf("Stack overflow at ip=%d\n", vm->ip);
        printf("Current stack size: %d\n", vm->sp);
        vm_debug_stack(vm);
        exit(1);
    }
    vm->stack[vm->sp++] = value;
}

Value vm_pop(VM* vm) {
    if (vm->sp <= 0) {
        printf("Stack underflow at ip=%d\n", vm->ip);
        printf("Current stack size: %d\n", vm->sp);
        vm_debug_stack(vm);
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
        printf("Unsupported types for ADD operation: %d and %d\n", a.type, b.type);
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
        printf("STORE_GLOBAL expects a string constant as variable name, but got type %d\n", name_val.type);
        exit(1);
    }
    scope_set(vm->scope, as_string(name_val), v);
}

static void op_load_global(VM* vm, int operand) {
    Value name_val = vm->bytecode->constants[operand];
    if (!is_obj_type(name_val, OBJ_STRING)) {
        printf("LOAD_GLOBAL expects a string constant as variable name, but got type %d\n", name_val.type);
        exit(1);
    }
    Value value = scope_find(vm->scope, as_string(name_val));
    if (value.type == VAL_NONE) {
        printf("Undefined global variable: %s\n", as_string(name_val)->chars);
        vm_debug_scope(vm);
        exit(1);
    }
    vm_push(vm, value);
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
        printf("Attempted to call a non-function value. Type: %d\n", func_val.type);
        exit(1);
    }
    if (vm->frame_count >= MAX_CALL_STACK_SIZE) {
        printf("Call stack overflow, %d > %d\n", vm->frame_count, MAX_CALL_STACK_SIZE);
        exit(1);
    }

    // Handle class instantiation
    if (func_val.as.object->type == OBJ_CLASS) {
        ObjClass* klass = (ObjClass*)func_val.as.object;
        
        // Create instance
        Value instance_val = vm_make_instance(vm, klass);
        
        // Look for __init__ method
        ObjString* init_name = intern_const_string(vm, "__init__", 8);
        Value init_method;
        int has_init = hash_get(klass->methods, init_name, &init_method);
        
        if (has_init && is_obj_type(init_method, OBJ_FUNCTION)) {
            // Call __init__ with instance as first argument
            ObjFunction* init_fn = (ObjFunction*)init_method.as.object;
            
            // Check parameter count
            if (init_fn->param_count != operand + 1) { // +1 for self
                printf("__init__ expects %d arguments but got %d\n", 
                       init_fn->param_count - 1, operand);
                exit(1);
            }
            
            // Create scope for __init__
            Scope* init_scope = new_scope("__init__", init_fn->scope ? init_fn->scope : vm->scope);
            
            // Bind self
            ObjString* self_param = intern_const_string(vm, init_fn->params[0], strlen(init_fn->params[0]));
            scope_set(init_scope, self_param, instance_val);
            
            // Bind other parameters
            for (int i = init_fn->param_count - 1; i > 0; i--) {
                Value arg_val = vm_pop(vm);
                const char* param_name = init_fn->params[i];
                ObjString* param_name_str = intern_const_string(vm, param_name, strlen(param_name));
                scope_set(init_scope, param_name_str, arg_val);
            }
            
            // Create call frame
            CallFrame* frame = &vm->call_stack[vm->frame_count++];
            frame->return_address = vm->ip;
            frame->scope = vm->scope;
            frame->base_sp = vm->sp;
            
            vm->scope = init_scope;
            vm->ip = init_fn->addr;
            
            // Store instance to return after __init__ completes
            vm->scope->return_value = instance_val;
        } else {
            // No __init__, just return the instance
            // Pop any arguments that were pushed
            for (int i = 0; i < operand; i++) {
                vm_pop(vm);
            }
            vm_push(vm, instance_val);
        }
        return;
    }

    if (func_val.as.object->type == OBJ_NATIVE_FUNCTION) {
        ObjNativeFunction* native_fn = (ObjNativeFunction*)func_val.as.object;
        // For simplicity, assume no arguments for native functions
        int arg_count = operand;
        Value result = native_fn->function(arg_count, &vm->stack[vm->sp - arg_count], vm);
        vm_push(vm, result);
        return;
    }

    ObjFunction* fn = (ObjFunction*)func_val.as.object;
    Scope* scope= new_scope(fn->name, fn->scope ? fn->scope : vm->scope);

    for (int i = fn->param_count - 1; i >= 0; i--) {
        Value arg_val = vm_pop(vm);
        const char* param_name = fn->params[i];
        ObjString* param_name_str = intern_const_string(vm, param_name, strlen(param_name));
        scope_set(scope, param_name_str, arg_val);
    }

    CallFrame* frame = &vm->call_stack[vm->frame_count++];
    frame->return_address = vm->ip;
    frame->scope = vm->scope;
    frame->base_sp = vm->sp;

    vm->scope = scope;
    vm->ip = fn->addr;
}

void op_return(VM* vm) {
    if (vm->frame_count <= 0) {
        printf("Call stack underflow\n");
        exit(1);
    }
    CallFrame* frame = &vm->call_stack[--vm->frame_count];
    
    Value ret_val = vm_pop(vm);
    
    // Check if this is returning from __init__
    // In that case, return the instance instead of None
    if (vm->scope->return_value.type != VAL_NONE) {
        ret_val = vm->scope->return_value;
    }
    
    if (vm->scope != frame->scope && frame->scope != NULL) {
        // Free current scope
        // free_scope(vm->scope);
        // Restore previous frame state
        vm->scope = frame->scope;
    }
    vm->sp = frame->base_sp;
    vm->ip = frame->return_address;
    vm_push(vm, ret_val);
}

static void op_index_get(VM* vm) {
    Value index_val = vm_pop(vm);
    Value list_val = vm_pop(vm);
    if (is_obj_type(list_val, OBJ_LIST)) {

        ObjList* list = (ObjList*)list_val.as.object;
        int index = (int)index_val.as.integer;
        if (index < 0 || index >= list->count) {
            printf("LIST_GET index out of bounds. Index: %d, List count: %d\n", index, list->count);
            exit(1);
        }
        Value item = list->items[index];
        vm_push(vm, item);
        return;
    }

    if (is_obj_type(list_val, OBJ_TUPLE)) {
        ObjTuple* tuple = (ObjTuple*)list_val.as.object;
        int index = (int)index_val.as.integer;
        if (index < 0 || index >= tuple->count) {
            printf("TUPLE_GET index out of bounds. Index: %d, Tuple count: %d\n", index, tuple->count);
            exit(1);
        }
        Value item = tuple->items[index];
        vm_push(vm, item);
        return;
    }

    if (is_obj_type(list_val, OBJ_DICT)) {
        ObjDict* dict = (ObjDict*)list_val.as.object;
        if (!is_obj_type(index_val, OBJ_STRING)) {
            printf("DICT_GET expects a string key, but got type %d\n", index_val.type);
            exit(1);
        }
        ObjString* key = as_string(index_val);
        Value val;
        if (hash_get(dict->map, key, &val)) {
            vm_push(vm, val);
            return;
        }
        printf("Key not found in dictionary: %s\n", key->chars);
        exit(1);
    }

    printf("IDX_GET expects a list, tuple, or dictionary value\n");
    exit(1);
}

static void op_index_set(VM* vm) {
    Value value = vm_pop(vm);
    Value index_val = vm_pop(vm);
    Value container = vm_pop(vm);
    if (is_obj_type(container, OBJ_LIST)) {

        ObjList* list = (ObjList*)container.as.object;
        int index = (int)index_val.as.integer;
        if (index < 0 || index >= list->count) {
            printf("LIST_SET index out of bounds. Index: %d, List count: %d\n", index, list->count);
            exit(1);
        }
        list->items[index] = value;
    }

    if (is_obj_type(container, OBJ_DICT)) {
        ObjDict* dict = (ObjDict*)container.as.object;
        if (!is_obj_type(index_val, OBJ_STRING)) {
            printf("DICT_SET expects a string key\n");
            exit(1);
        }

        ObjString* key = as_string(index_val);

        hash_set(dict->map, key, value);

        return;
    }
    printf("IDX_SET expects a list or dictionary value\n");
    exit(1);
}

static void op_make_class(VM* vm, int operand) {
    // Stack: [parent_class or None]
    // operand: index of class name in constants
    Value parent_val = vm_pop(vm);
    ObjClass* parent = NULL;
    
    if (parent_val.type == VAL_OBJ && parent_val.as.object->type == OBJ_CLASS) {
        parent = (ObjClass*)parent_val.as.object;
    }
    
    ObjString* class_name = as_string(vm->bytecode->constants[operand]);
    Value class_val = vm_make_class(vm, class_name->chars, parent);
    
    vm_push(vm, class_val);
}

static void op_make_instance(VM* vm) {
    // Stack: [class, ...args]
    Value class_val = vm_pop(vm);
    
    if (!is_obj_type(class_val, OBJ_CLASS)) {
        printf("Can only instantiate classes\n");
        exit(1);
    }
    
    ObjClass* klass = (ObjClass*)class_val.as.object;
    Value instance_val = vm_make_instance(vm, klass);
    
    vm_push(vm, instance_val);
}

static void op_get_attr(VM* vm, int operand) {
    // Stack: [object]
    // operand: index of attribute name in constants
    Value obj_val = vm_pop(vm);
    ObjString* attr_name = as_string(vm->bytecode->constants[operand]);
    
    if (is_obj_type(obj_val, OBJ_INSTANCE)) {
        ObjInstance* instance = (ObjInstance*)obj_val.as.object;
        
        // Try to find field
        Value field_val;
        if (hash_get(instance->fields, attr_name, &field_val)) {
            vm_push(vm, field_val);
            return;
        }
        
        // Try to find method in class
        Value method_val;
        if (hash_get(instance->klass->methods, attr_name, &method_val)) {
            vm_push(vm, method_val);
            return;
        }
        
        // Try parent classes
        ObjClass* parent = instance->klass->parent;
        while (parent) {
            if (hash_get(parent->methods, attr_name, &method_val)) {
                vm_push(vm, method_val);
                return;
            }
            parent = parent->parent;
        }
        
        printf("Attribute '%s' not found on instance\n", attr_name->chars);
        exit(1);
    } else if (is_obj_type(obj_val, OBJ_CLASS)) {
        ObjClass* klass = (ObjClass*)obj_val.as.object;
        
        // Try to find method
        Value method_val;
        if (hash_get(klass->methods, attr_name, &method_val)) {
            vm_push(vm, method_val);
            return;
        }
        
        printf("Attribute '%s' not found on class\n", attr_name->chars);
        exit(1);
    }
    
    printf("GET_ATTR expects an instance or class\n");
    exit(1);
}

static void op_set_attr(VM* vm, int operand) {
    // Stack: [object, value]
    // operand: index of attribute name in constants
    Value value = vm_pop(vm);
    Value obj_val = vm_pop(vm);
    ObjString* attr_name = as_string(vm->bytecode->constants[operand]);
    
    if (is_obj_type(obj_val, OBJ_INSTANCE)) {
        ObjInstance* instance = (ObjInstance*)obj_val.as.object;
        hash_set(instance->fields, attr_name, value);
        return;
    } else if (is_obj_type(obj_val, OBJ_CLASS)) {
        ObjClass* klass = (ObjClass*)obj_val.as.object;
        // Setting methods on class
        hash_set(klass->methods, attr_name, value);
        return;
    }
    
    printf("SET_ATTR expects an instance or class\n");
    exit(1);
}

static void op_call_method(VM* vm, int operand) {
    // Stack: [object, arg1, arg2, ...]
    // operand: method name index
    // Next instruction should be NOP with argc
    
    ObjString* method_name = as_string(vm->bytecode->constants[operand]);
    
    // Get argc from next instruction  
    Instruction next_instr = vm->bytecode->instructions[vm->ip++];
    int argc = next_instr.operand;
    
    // Get the object (it's at position sp - argc - 1)
    Value obj_val = vm->stack[vm->sp - argc - 1];
    
    if (!is_obj_type(obj_val, OBJ_INSTANCE)) {
        printf("Can only call methods on instances\n");
        exit(1);
    }
    
    ObjInstance* instance = (ObjInstance*)obj_val.as.object;
    
    // Look up method
    Value method_val;
    int found = 0;
    
    if (hash_get(instance->klass->methods, method_name, &method_val)) {
        found = 1;
    } else {
        // Try parent classes
        ObjClass* parent = instance->klass->parent;
        while (parent && !found) {
            if (hash_get(parent->methods, method_name, &method_val)) {
                found = 1;
                break;
            }
            parent = parent->parent;
        }
    }
    
    if (!found) {
        printf("Method '%s' not found\n", method_name->chars);
        exit(1);
    }
    
    if (!is_obj_type(method_val, OBJ_FUNCTION)) {
        printf("Method is not a function\n");
        exit(1);
    }
    
    ObjFunction* fn = (ObjFunction*)method_val.as.object;
    
    // Check parameter count (should be argc + 1 for 'self')
    if (fn->param_count != argc + 1) {
        printf("Method '%s' expects %d arguments but got %d\n", 
               method_name->chars, fn->param_count - 1, argc);
        exit(1);
    }
    
    // Create new call frame - object is already on stack as first arg
    CallFrame* frame = &vm->call_stack[vm->frame_count++];
    frame->return_address = vm->ip;
    frame->base_sp = vm->sp - argc - 1; // Points to object
    frame->scope = vm->scope;  // Save current scope to restore later
    
    // Create new scope for the method
    Scope* method_scope = new_scope(fn->name, vm->scope);
    
    // Bind parameters to arguments
    for (int i = 0; i < fn->param_count; i++) {
        Value arg = vm->stack[frame->base_sp + i];
        ObjString* param_name = intern_const_string(vm, fn->params[i], strlen(fn->params[i]));
        scope_set(method_scope, param_name, arg);
    }
    
    vm->scope = method_scope;
    vm->sp = frame->base_sp; // Reset stack to base
    vm->ip = fn->addr; // Jump to function
}
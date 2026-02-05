#include "native_func.h"

#include "hashmap.h"
#include "vm.h"
#include "vm_objects.h"
#include "gc.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

Value native_print(int arg_count, Value* args, VM* vm) {
    for (int i = 0; i < arg_count; i++) {
        print_value(args[i]);
        if (i < arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    return make_none();
}

Value native_len(int arg_count, Value* args, VM* vm) {
    if (arg_count != 1) {
        printf("len() takes exactly one argument (%d given)\n", arg_count);
        exit(1);
    }
    Value arg = args[0];
    if (arg.type != VAL_OBJ) {
        printf("len() argument must be a container or string\n");
        exit(1);
    }

    if (arg.as.object->type == OBJ_LIST) {
        ObjList* list = (ObjList*)arg.as.object;
        Value result = {0};
        result.type = VAL_INT;
        result.as.integer = list->count;
        return result;
    }

    if (arg.as.object->type == OBJ_STRING) {
        ObjString* str = (ObjString*)arg.as.object;
        Value result = {0};
        result.type = VAL_INT;
        result.as.integer = strlen(str->chars);
        return result;
    }
    printf("len() argument must be a container or string\n");
    exit(1);
}

Value native_clock(int arg_count, Value* args, VM* vm) {
    if (arg_count != 0) {
        printf("clock() takes no arguments (%d given)\n", arg_count);
        exit(1);
    }
    Value result = {0};
    result.type = VAL_FLOAT;
    result.as.floating = (double)clock() / CLOCKS_PER_SEC;
    return result;
}

Value native_input(int arg_count, Value* args, VM* vm) {
    if (arg_count > 1) {
        printf("input() takes at most one argument (%d given)\n", arg_count);
        exit(1);
    }
    if (arg_count == 1) {
        print_value(args[0]);
    }
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Error reading input\n");
        exit(1);
    }
    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return make_string(buffer);
}

Value native_exit(int arg_count, Value* args, VM* vm) {
    exit(0);
    return make_none(); // Unreachable
}

Value native_type(int arg_count, Value* args, VM* vm) {
    if (arg_count != 1) {
        printf("type() takes exactly one argument (%d given)\n", arg_count);
        exit(1);
    }
    Value arg = args[0];
    const char* type_name;
    switch (arg.type) {
        case VAL_INT: type_name = "int"; break;
        case VAL_FLOAT: type_name = "float"; break;
        case VAL_BOOL: type_name = "bool"; break;
        case VAL_NONE: type_name = "NoneType"; break;
        case VAL_OBJ:
            if (arg.as.object->type == OBJ_STRING) {
                type_name = "str";
            } else if (arg.as.object->type == OBJ_LIST) {
                type_name = "list";
            } else if (arg.as.object->type == OBJ_DICT) {
                type_name = "dict";
            } else if (arg.as.object->type == OBJ_TUPLE) {
                type_name = "tuple";
            } else if (arg.as.object->type == OBJ_SET) {
                type_name = "set";
            } else if (arg.as.object->type == OBJ_FUNCTION) {
                type_name = "function";
            } else if (arg.as.object->type == OBJ_NATIVE_FUNCTION) {
                type_name = "native_function";
            } else if (arg.as.object->type == OBJ_CLASS) {
                type_name = "class";
            } else if (arg.as.object->type == OBJ_INSTANCE) {
                ObjInstance* instance = (ObjInstance*)arg.as.object;
                type_name = instance->klass->name;
            } else {
                type_name = "<unknown object>";
            }
        break;
        default:
            type_name = "<unknown>";
    }
    printf("<type '%s'>\n", type_name);
    return make_string(type_name);
}

Value native_int(int arg_count, Value* args, VM* vm) {
    if (arg_count != 1) {
        printf("int() takes exactly one argument (%d given)\n", arg_count);
        exit(1);
    }
    Value arg = args[0];
    if (arg.type == VAL_INT) {
        return arg;
    } else if (arg.type == VAL_FLOAT) {
        return make_number_int((int)arg.as.floating);
    } else if (is_obj_type(arg, OBJ_STRING)) {
        ObjString* str = as_string(arg);
        char* endptr;
        long val = strtol(str->chars, &endptr, 10);
        if (*endptr != '\0') {
            printf("Cannot convert string to int: '%s'\n", str->chars);
            exit(1);
        }
        return make_number_int(val);
    } else {
        printf("int() argument must be a number or string\n");
        exit(1);
    }
}

Value native_float(int arg_count, Value* args, VM* vm) {
    if (arg_count != 1) {
        printf("float() takes exactly one argument (%d given)\n", arg_count);
        exit(1);
    }
    Value arg = args[0];
    if (arg.type == VAL_FLOAT) {
        return arg;
    } else if (arg.type == VAL_INT) {
        return make_number_float((double)arg.as.integer);
    } else if (is_obj_type(arg, OBJ_STRING)) {
        ObjString* str = as_string(arg);
        char* endptr;
        double val = strtod(str->chars, &endptr);
        if (*endptr != '\0') {
            printf("Cannot convert string to float: '%s'\n", str->chars);
            exit(1);
        }
        return make_number_float(val);
    } else {
        printf("float() argument must be a number or string\n");
        exit(1);
    }
}

Value native_str(int arg_count, Value* args, VM* vm) {
    if (arg_count != 1) {
        printf("str() takes exactly one argument (%d given)\n", arg_count);
        exit(1);
    }
    Value arg = args[0];
    if (arg.type == VAL_OBJ && arg.as.object->type == OBJ_STRING) {
        return arg;
    } else if (arg.type == VAL_INT) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%ld", arg.as.integer);
        return make_string(buffer);
    } else if (arg.type == VAL_FLOAT) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%g", arg.as.floating);
        return make_string(buffer);
    } else if (arg.type == VAL_BOOL) {
        return make_string(arg.as.boolean ? "True" : "False");
    } else if (arg.type == VAL_NONE) {
        return make_string("None");
    } else if (arg.type == VAL_OBJ) {
        // For simplicity, just return a placeholder string for objects
        return make_string("<object>");
    } else {
        printf("str() argument has unsupported type\n");
        exit(1);
    }
}

Value native_gc_collect(int arg_count, Value* args, VM* vm) {
    if (arg_count != 0) {
        printf("gc_collect() takes no arguments (%d given)\n", arg_count);
        exit(1);
    }
    #if VM_USE_GC
    gc_collect(vm);
    #endif
    return make_none();
}

Value native_gc_stats(int arg_count, Value* args, VM* vm) {
    if (arg_count != 0) {
        printf("gc_stats() takes no arguments (%d given)\n", arg_count);
        exit(1);
    }
    Value result = {0};
    result.type = VAL_OBJ;
    ObjDict* dict = malloc(sizeof(ObjDict));
    dict->obj.type = OBJ_DICT;
    dict->count = 0;
    dict->capacity = 4;
    dict->map = malloc(sizeof(HashMap));
    hash_init(dict->map, 4);

    // Add stats
    Value allocated = {0};
    allocated.type = VAL_INT;
    allocated.as.integer = vm->bytes_allocated;
    ObjString* key_allocated = malloc(sizeof(ObjString));
    key_allocated->obj.type = OBJ_STRING;
    key_allocated->chars = strdup("allocated_bytes");
    key_allocated->length = strlen(key_allocated->chars);
    hash_set(dict->map, key_allocated, allocated);

    Value next_gc = {0};
    next_gc.type = VAL_INT;
    #if VM_USE_GC
        next_gc.as.integer = vm->next_gc;
    #else
        next_gc.as.integer = -1; // GC not enabled
    #endif
    ObjString* key_next_gc = (ObjString*)vm_make_string(vm, "next_gc_bytes").as.object;
    key_next_gc->obj.type = OBJ_STRING;
    key_next_gc->chars = strdup("next_gc_bytes");
    key_next_gc->length = strlen(key_next_gc->chars);
    hash_set(dict->map, key_next_gc, next_gc);

    result.as.object = (Obj*)dict;
    return result;
}

Value native_make_list(int arg_count, Value* args, VM* vm) {
    Value list_val = vm_make_list(vm, arg_count);
    ObjList* list = (ObjList*)list_val.as.object;
    list->count = arg_count;

    for (int i = arg_count - 1; i >= 0; i--) {
        Value val = vm_pop(vm);
        list->items[i] = val;
    }

    return list_val;
}

Value native_make_dict(int arg_count, Value* args, VM* vm) {
    Value dict_val = vm_make_dict(vm);
    ObjDict* dict = (ObjDict*)dict_val.as.object;
    dict->count = arg_count;

    for (int i = arg_count - 1; i >= 0; i--) {
        Value val = vm_pop(vm);
        Value key = vm_pop(vm);

        if (!is_obj_type(key, OBJ_STRING)) {
            printf("Dictionary keys must be strings\n");
            exit(1);
        }
        hash_set(dict->map, as_string(key), val);
    }

    return dict_val;
}

Value native_make_set(int arg_count, Value* args, VM* vm) {
    Value set_val = vm_make_set(vm);
    ObjSet* set = (ObjSet*)set_val.as.object;
    set->count = arg_count;

    for (int i = arg_count - 1; i >= 0; i--) {
        Value val = vm_pop(vm);

        // Create string representation of the value to use as key
        char key[64];
        if (val.type == VAL_INT) {
            snprintf(key, sizeof(key), "%ld", val.as.integer);
        } else if (val.type == VAL_FLOAT) {
            snprintf(key, sizeof(key), "%g", val.as.floating);
        } else if (is_obj_type(val, OBJ_STRING)) {
            snprintf(key, sizeof(key), "%s", as_string(val)->chars);
        } else {
            snprintf(key, sizeof(key), "obj_%p", (void*)val.as.object);
        }
        
        ObjString* key_str = malloc(sizeof(ObjString));
        key_str->obj.type = OBJ_STRING;
        key_str->chars = strdup(key);
        key_str->length = strlen(key);
        vm->bytes_allocated += sizeof(ObjString) + strlen(key) + 1;
        
        hash_set(set->map, key_str, val);
    }

    vm_push(vm, set_val);
    return set_val;
}

Value native_make_tuple(int arg_count, Value* args, VM* vm) {
    Value tuple_val = vm_make_tuple(vm);
    ObjTuple* tuple = (ObjTuple*)tuple_val.as.object;
    tuple->count = arg_count;
    tuple->items = malloc(sizeof(Value) * arg_count);
    vm->bytes_allocated += sizeof(Value) * arg_count;

    for (int i = arg_count - 1; i >= 0; i--) {
        Value val = vm_pop(vm);
        tuple->items[i] = val;
    }

    return tuple_val;
}
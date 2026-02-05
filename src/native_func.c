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

Value native_gc_collect(int arg_count, Value* args, VM* vm) {
    if (arg_count != 0) {
        printf("gc_collect() takes no arguments (%d given)\n", arg_count);
        exit(1);
    }
    gc_collect(vm);
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
    next_gc.as.integer = vm->next_gc;
    ObjString* key_next_gc = malloc(sizeof(ObjString));
    key_next_gc->obj.type = OBJ_STRING;
    key_next_gc->chars = strdup("next_gc_bytes");
    key_next_gc->length = strlen(key_next_gc->chars);
    hash_set(dict->map, key_next_gc, next_gc);

    result.as.object = (Obj*)dict;
    return result;
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
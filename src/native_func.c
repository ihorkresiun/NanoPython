#include "native_func.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

Value native_print(int arg_count, Value* args) {
    for (int i = 0; i < arg_count; i++) {
        print_value(args[i]);
        if (i < arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    return make_none();
}

Value native_len(int arg_count, Value* args) {
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

Value native_clock(int arg_count, Value* args) {
    if (arg_count != 0) {
        printf("clock() takes no arguments (%d given)\n", arg_count);
        exit(1);
    }
    Value result = {0};
    result.type = VAL_FLOAT;
    result.as.floating = (double)clock() / CLOCKS_PER_SEC;
    return result;
}

Value native_exit(int arg_count, Value* args) {
    exit(0);
    return make_none(); // Unreachable
}
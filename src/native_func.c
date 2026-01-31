#include "native_func.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

ObjNativeFunction* make_native_function(const char* name, NativeFn function) {
    ObjNativeFunction* native_fn = malloc(sizeof(ObjNativeFunction));
    native_fn->obj.type = OBJ_NATIVE_FUNCTION;
    native_fn->function = function;
    native_fn->name = strdup(name);
    return native_fn;
}

void register_native_functions(Scope* scope, const char* name, NativeFn function) {
    ObjNativeFunction* native_fn = make_native_function(name, function);
    Value native_fn_val = {0};
    native_fn_val.type = VAL_OBJ;
    native_fn_val.as.object = (Obj*)native_fn;
    scope_set(scope, name, native_fn_val);
}

Value native_helllo(int arg_count, Value* args) {
    printf("Hello from native function!\n");
    return make_none();
}

Value native_with_args(int arg_count, Value* args) {
    printf("Native function called with %d arguments:\n", arg_count);
    for (int i = 0; i < arg_count; i++) {
        printf(" Arg %d: ", i);
        print_value(args[i]);
        printf("\n");
    }
    return make_none();
}

Value native_factorial(int arg_count, Value* args) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        printf("factorial expects one integer argument\n");
        return make_none();
    }

    long n = args[0].as.integer;
    if (n < 0) {
        printf("factorial not defined for negative numbers\n");
        return make_none();
    }

    long result = 1;
    for (long i = 2; i <= n; i++) {
        result *= i;
    }

    return make_number_int(result);
}
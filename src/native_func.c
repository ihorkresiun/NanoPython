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
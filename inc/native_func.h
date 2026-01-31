#ifndef __INC_NATIVE_FUNC_H__
#define __INC_NATIVE_FUNC_H__

#include "vars.h"

ObjNativeFunction* make_native_function(const char* name, NativeFn function);
void register_native_functions(Scope* scope, const char* name, NativeFn function);

Value native_print(int arg_count, Value* args);

#endif // __INC_NATIVE_FUNC_H__

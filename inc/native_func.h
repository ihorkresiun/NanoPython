#ifndef __INC_NATIVE_FUNC_H__
#define __INC_NATIVE_FUNC_H__

#include "vars.h"

Value native_print(int arg_count, Value* args);
Value native_len(int arg_count, Value* args);
Value native_clock(int arg_count, Value* args);
Value native_exit(int arg_count, Value* args);

#endif // __INC_NATIVE_FUNC_H__

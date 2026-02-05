#ifndef __INC_NATIVE_FUNC_H__
#define __INC_NATIVE_FUNC_H__

#include "vars.h"

typedef struct VM VM; // forward declaration

Value native_print(int arg_count, Value* args, VM* vm);
Value native_len(int arg_count, Value* args, VM* vm);
Value native_clock(int arg_count, Value* args, VM* vm);
Value native_exit(int arg_count, Value* args, VM* vm);
Value native_input(int arg_count, Value* args, VM* vm);
Value native_gc_collect(int arg_count, Value* args, VM* vm);
Value native_gc_stats(int arg_count, Value* args, VM* vm);

Value native_make_dict(int arg_count, Value* args, VM* vm);

#endif // __INC_NATIVE_FUNC_H__

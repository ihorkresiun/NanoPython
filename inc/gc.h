#ifndef __INC_VM_GC_H__
#define __INC_VM_GC_H__

#include "vars.h"

typedef struct VM VM;

void gc_mark(VM* vm, Value value);
void gc_mark_all(VM* vm);
void gc_sweep(VM* vm);
void gc_collect(VM* vm);

#endif // __INC_VM_GC_H__

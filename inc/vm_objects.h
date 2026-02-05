#ifndef __INC_VM_OBJECTS_H__
#define __INC_VM_OBJECTS_H__

#include "vars.h"
#include "vm.h"

#include "stdint.h"
#include "stddef.h"

Obj* vm_alloc_object(VM* vm, size_t size, ObjectType type);
Value vm_make_string(VM* vm, const char* s);
Value vm_make_list(VM* vm, int count);
Value vm_make_dict(VM* vm);
Value vm_make_tuple(VM* vm);
Value vm_make_set(VM* vm);
Value vm_make_class(VM* vm, const char* name, ObjClass* parent);
Value vm_make_instance(VM* vm, ObjClass* klass);

#endif // __INC_VM_OBJECTS_H__

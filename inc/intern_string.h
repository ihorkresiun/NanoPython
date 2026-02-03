#ifndef __INC_INTERN_STRING_H__
#define __INC_INTERN_STRING_H__

#include "vars.h"
#include "vm.h"

ObjString* intern_string(VM* vm, char* chars, int length);
ObjString* intern_const_string(VM* vm, const char* chars, int length);

#endif // __INC_INTERN_STRING_H__

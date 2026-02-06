#ifndef __INC_BYTECODE_H__
#define __INC_BYTECODE_H__

#include "vm.h"

// Store Bytecode instructions and constants to a file for later execution
int bytecode_serialize(Bytecode* bytecode, const char* filename);

// Load Bytecode instructions and constants from a file
Bytecode* bytecode_deserialize(const char* filename);

// Disassemble Bytecode instructions and constants to a file
int bytecode_disasm(Bytecode* bytecode, const char* filename);

#endif /* __INC_BYTECODE_H__ */

#include "stdio.h"

#include "bytecode.h"
#include "native_func.h"
#include "vm.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    const char* source_file = argv[1];

    Bytecode* bytecode = bytecode_deserialize(source_file);
    if (!bytecode) {
        printf("Error: Failed to load bytecode from file '%s'\n", source_file);
        return 1;
    }

    VM vm;
    vm_init(&vm, bytecode);
    register_native_functions(&vm);
    vm_run(&vm);
    
    free(bytecode->instructions);
    free(bytecode->constants);
    free(bytecode);

    return 0;
}

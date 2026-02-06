#include "stdio.h"

#include "bytecode.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <out_file>\n", argv[0]);
        return 1;
    }

    const char* source_file = argv[1];

    Bytecode* bytecode = bytecode_deserialize(source_file);
    if (!bytecode) {
        printf("Error: Failed to load bytecode from file '%s'\n", source_file);
        return 1;
    }

    const char* out_file = argv[2];
    if (!bytecode_disasm(bytecode, out_file)) {
        printf("Error: Failed to disassemble bytecode to file '%s'\n", out_file);
        return 1;
    }

    printf("Bytecode disassembled to file '%s'\n", out_file);

    return 0;
}

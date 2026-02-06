#include "stdio.h"

#include "ast.h"
#include "bytecode.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <source_file> <bytecode_file>\n", argv[0]);
        return 1;
    }

    const char* source_file = argv[1];
    char* source = NULL;
    FILE* file = fopen(source_file, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        source = malloc(fsize + 1);
        fread(source, 1, fsize, file);
        source[fsize] = 0;
        fclose(file);
    } else {
        printf("Error: Could not open file %s\n", source_file);
        return 1;
    }

    Lexer lexer = {0};
    Parser parser = {&lexer, {0}};

    lexer_init(&lexer, source);
    parser_init(&parser, source);
    Ast* tree = parse_program(&parser);

    if (!tree) {
        printf("Error: Failed to parse program.\n");
        return 1;
    }
    
    Compiler compiler;
    compiler_init(&compiler);
    Bytecode* bytecode = compile(&compiler, tree);
    if (!bytecode) {
        printf("Error: Compilation failed.\n");
        return 1;
    }

    const char* bytecode_file = argv[2];
    int serialize_success = bytecode_serialize(bytecode, bytecode_file);
    if (!serialize_success) {
        printf("Error: Failed to serialize bytecode.\n");
        return 1;
    }

    printf("Bytecode serialized to '%s'\n", bytecode_file);

    free(source);
    free(bytecode->instructions);
    free(bytecode->constants);
    free(bytecode);

    return 0;
}

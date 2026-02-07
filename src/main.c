#include "stdio.h"

#include "ast.h"
#include "bytecode.h"
#include "compiler.h"
#include "lexer.h"
#include "native_func.h"
#include "np_config.h"
#include "parser.h"
#include "vm.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

static int mode_repl();
static int mode_file(const char* source_file);

int main(int argc, char** argv) {
    if (argc == 1) {
        return mode_repl();
    } else if (argc == 2) {
        return mode_file(argv[1]);
    }

    return 0;
}

static int mode_repl() {
    // REPL mode
    printf("NanoPython REPL v%s\n", NP_VERSION);
    printf("Type 'exit()' to quit\n\n");
    
    char line[1024];
    VM vm;
    int vm_initialized = 0;
    
    while (1) {
        printf(">>> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse and execute
        Lexer lexer = {0};
        Parser parser = {&lexer, {0}};
        
        lexer_init(&lexer, line);
        parser_init(&parser, line);
        Ast* tree = parse_program(&parser);
        
        if (!tree) {
            printf("Syntax error\n");
            continue;
        }
        
        Compiler compiler;
        compiler_init(&compiler);
        Bytecode* bytecode = compile(&compiler, tree);
        
        if (!bytecode) {
            printf("Compilation error\n");
            ast_free(tree);
            continue;
        }

        if (NP_DEBUG > 0) {
            bytecode_disasm(bytecode, "bytecode.txt");
        }
        
        // Initialize VM on first use, otherwise reuse it
        if (!vm_initialized) {
            vm_init(&vm, bytecode);
            register_native_functions(&vm);
            vm_initialized = 1;
        }
        
        // Update VM to use new bytecode
        vm.bytecode = bytecode;
        vm.ip = 0;
        
        // Execute and check if value left on stack
        int old_sp = vm.sp;
        vm_run(&vm);
        
        // If expression left a value, print it
        if (vm.sp > old_sp && vm.sp > 0) {
            print_value(vm.stack[vm.sp - 1]);
            printf("\n");
            vm.sp--;  // Pop the result
        }
        
        ast_free(tree);
    }
}

static int mode_file(const char* source_file) {
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

    if (NP_DEBUG > 0) {
        ast_dump(tree, "ast_dump.txt");
    }

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
    bytecode_disasm(bytecode, "bytecode.txt");

    VM vm;
    vm_init(&vm, bytecode);
    register_native_functions(&vm);
    vm_run(&vm);
    free(source);
}

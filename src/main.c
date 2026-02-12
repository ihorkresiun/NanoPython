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
    printf("NanoPython REPL v%s\n", NP_VERSION);
    printf("Type 'exit()' to quit\n\n");
    
    char buffer[1024];
    VM vm;
    Compiler compiler;
    int vm_initialized = 0;
    
    // Initialize compiler once - it will accumulate bytecode
    compiler_init(&compiler);
    
    while (1) {
        printf(">>> ");
        fflush(stdout);
        
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        
        int len = strlen(buffer);
        // Skip empty lines
        if (len == 0) {
            continue;
        }
    
        if (buffer[len - 2] == ':') {
            // Multi-line block mode

            char line[128];
            
            while (1) {
                printf("... ");
                fflush(stdout);
                
                if (!fgets(line, sizeof(line), stdin)) {
                    break;
                }

                strcat(buffer, line);

                // Empty line ends block
                if (line[0] == '\n') {
                    break;
                }
            }
        }
        
        // Parse and execute
        Lexer lexer = {0};
        Parser parser = {&lexer, {0}};
        
        lexer_init(&lexer, buffer);
        parser_init(&parser, buffer);
        
        Ast* tree = parse_statement(&parser);
        
        if (!tree) {
            printf("Syntax error\n");
            continue;
        }
        
        // Compile using the same compiler instance (accumulates bytecode)
        // Start IP to execute new portion of bytecode is current bytecode count
        int start_ip = compiler.bytecode->count;
        Bytecode* bytecode = compile(&compiler, tree);

        if (NP_DEBUG > 0) {
            bytecode_disasm(bytecode, "repl_bytecode.txt");
        }
        
        if (!bytecode) {
            printf("Compilation error\n");
            ast_free(tree);
            continue;
        }
        
        // Initialize VM on first use
        if (!vm_initialized) {
            vm_init(&vm, bytecode);
            register_native_functions(&vm);
            vm_initialized = 1;
        } else {
            // VM already has reference to the same bytecode
            // Just reset IP to execute new instructions
            vm.ip = start_ip;
        }
        
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
    
    printf("Goodbye!\n");
    return 0;
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
    
    if (NP_DEBUG > 0) {
        bytecode_disasm(bytecode, "bytecode.txt");
    }

    ast_free(tree);
    free(source);

    VM vm;
    vm_init(&vm, bytecode);
    register_native_functions(&vm);

    vm_run(&vm);

    compiler_free(&compiler);

    return 0;
}

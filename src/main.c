#include "stdio.h"

#include "ast.h"
#include "compiler.h"
#include "disasm.h"
#include "eval.h"
#include "lexer.h"
#include "native_func.h"
#include "parser.h"
#include "vm.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

static void register_native_functions(VM* vm) {
    vm_register_native_functions(vm, "print", native_print);
    vm_register_native_functions(vm, "len", native_len);
    vm_register_native_functions(vm, "time", native_clock);
    vm_register_native_functions(vm, "exit", native_exit);
    vm_register_native_functions(vm, "input", native_input);
    vm_register_native_functions(vm, "gc", native_gc_collect);
    vm_register_native_functions(vm, "mem", native_gc_stats);
    vm_register_native_functions(vm, "native_make_dict", native_make_dict);
    vm_register_native_functions(vm, "native_make_list", native_make_list);
    vm_register_native_functions(vm, "native_make_set", native_make_set);
    vm_register_native_functions(vm, "native_make_tuple", native_make_tuple);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        // REPL mode
        printf("NanoPython REPL v0.1\n");
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
        
        return 0;
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
    store_disasm(bytecode, "bytecode.txt");

    VM vm;
    vm_init(&vm, bytecode);
    register_native_functions(&vm);
    vm_run(&vm);
    free(source);
}

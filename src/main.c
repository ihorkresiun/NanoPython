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


int main(int argc, char** argv) {
    if (argc < 2) {
        // REPL mode or usage message
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
    vm_register_native_functions(&vm, "print", native_print);
    vm_register_native_functions(&vm, "len", native_len);
    vm_register_native_functions(&vm, "time", native_clock);
    vm_register_native_functions(&vm, "exit", native_exit);
    vm_run(&vm);
    free(source);
}

#if 0
int main_vm(int argc, char** argv) {
    static const Instruction code[] = {
        {OP_CONST, 0}, // Push constant 0 (placeholder)
        {OP_CONST, 1}, // Push constant 1 (placeholder)
        {OP_ADD, 0},  // Add top two values
        {OP_PRINT, 0}, // Print result
        {OP_HALT, 0}   // Halt execution
    };
    static const Value constants[] = {
        {.type = VAL_FLOAT, .as.f = 3},
        {.type = VAL_FLOAT, .as.f = 4}
    };

    Bytecode bytecode = {
        .instructions = (Instruction*)code,
        .count = sizeof(code) / sizeof(Instruction),
        .capacity = sizeof(code) / sizeof(Instruction),
        .constants = (Value*)constants,
        .const_count = sizeof(constants) / sizeof(Value)
    };

    Scope globals = {
        .name = "Global",
        .vars = NULL,
        .parent = NULL
    };

    VM vm = {
        .bytecode = &bytecode,
        .sp = 0,
        .ip = 0,
        .scope = &globals
    };

    vm_run(&vm);
}

int main_ast(int argc, char** argv)
{
    char line[128] = {0};
    char buffer[2048] = {0};

    static const char* welcome_msg = 
        "NanoPython Calculator\n"
        "Type 'exit' or 'quit' to leave.\n";

    printf("%s", welcome_msg);

    static const char* prompt = ">>> ";

    Lexer lexer = {0};
    Parser parser = {&lexer, {0}};
    Scope global_scope = {0};
    global_scope.name = "Global";
    global_scope.vars = NULL;
    global_scope.parent = NULL;
    global_scope.return_value = make_none();
    
    while (1)
    {
        // REPL loop
        // Todo: handle file input from argv[1]
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) break;

        if (strcmp(line, "exit\n") == 0 || strcmp(line, "quit\n") == 0) {
            break;
        }
        
        size_t len = strlen(line);

        strcat(buffer, line);

        if (strchr(buffer, ':')) {
            while (1) {
                printf("... ");
                if (!fgets(line, sizeof(line), stdin)) break;
                size_t l = strlen(line);
                if (strlen(line) == 0 || line[0] == '\n' || feof(stdin)) break;
                strcat(buffer, line);
            }
        }

        lexer_init(&lexer, buffer);
        parser_init(&parser, buffer);
        Ast* tree = parse_statement(&parser);
        if (!tree) {
            printf("Error: Failed to parse expression.\n");
            continue;
        }

        EvalResult res = eval(tree, &global_scope);
        if (res.status != NORMAL) {
            printf("Error: Evaluation failed.\n");
            continue;
        }

        print_value(res.value);
        printf("\n");
        
        ast_free(tree);
        buffer[0] = 0; // Clear buffer for next input
    }

    return 0;
}

#endif
#include "stdio.h"

#include "ast.h"
#include "compiler.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"

#include "string.h"

static void print_usage(const char* prog_name) {
    printf("Usage: %s \"expression\"\n", prog_name);
    printf("Evaluates the arithmetic expression provided as a command-line argument.\n");
    printf("Example: %s \"3 + 4 * (2 - 1)\"\n", prog_name);
}   

int main(int argc, char** argv) {
    const char* source = "x = 10\nx=x+4\nprint(x + 12*2)\0";
    //const char* source = "x = 10 + 2 * 3\nprint(x)\0";

    Lexer lexer = {0};
    Parser parser = {&lexer, {0}};

    lexer_init(&lexer, source);
    parser_init(&parser, source);
    Ast* tree = parse_program(&parser);

    static Instruction code[64] = {0};
    static Value constants[16] = {0};
    static Bytecode bytecode = {
        .instructions = code,
        .count = 0,
        .capacity = 64,
        .constants = constants,
        .const_count = 0
    };

    Compiler compiler = {&bytecode};
    compile(&compiler, tree);

    Scope globals = {0};
    globals.name = "Global";
    VM vm = {&bytecode, {0}, 0, 0, &globals};
    vm_run(&vm);
}

int main_vm(int argc, char** argv) {
    static const Instruction code[] = {
        {OP_CONST, 0}, // Push constant 0 (placeholder)
        {OP_CONST, 1}, // Push constant 1 (placeholder)
        {OP_ADD, 0},  // Add top two values
        {OP_PRINT, 0}, // Print result
        {OP_HALT, 0}   // Halt execution
    };
    static const Value constants[] = {
        {.type = VAL_FLOAT, .value.f = 3},
        {.type = VAL_FLOAT, .value.f = 4}
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
        .globals = &globals
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
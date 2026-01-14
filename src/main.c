#include "stdio.h"

#include "ast.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"

#include "string.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s \"expression\"\n", prog_name);
    printf("Evaluates the arithmetic expression provided as a command-line argument.\n");
    printf("Example: %s \"3 + 4 * (2 - 1)\"\n", prog_name);
}   

int main(int argc, char** argv)
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
    Scope global_scope = {"Global", NULL, NULL};
    
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

        parser_init(&parser, buffer);
        Ast* tree = parse_statement(&parser);
        if (!tree) {
            printf("Error: Failed to parse expression.\n");
            continue;
        }
        
        Value result = eval(tree, &global_scope);
        if (result.type == VAL_NUMBER) {
            printf("= %g\n", result.value.number);
        } else if (result.type == VAL_BOOL) {
            printf("= %s\n", result.value.boolean ? "True" : "False");
        } else if (result.type == VAL_NONE) {
            printf("=\n");
        }

        ast_free(tree);
        buffer[0] = 0; // Clear buffer for next input
    }

    return 0;
}
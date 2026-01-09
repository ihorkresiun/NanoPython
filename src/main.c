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
    char input[256] = {0};
    static const char* welcome_msg = 
        "NanoPython Calculator\n"
        "Type 'exit' or 'quit' to leave.\n";

    printf("%s", welcome_msg);

    static const char* prompt = ">>> ";

    Lexer lexer;
    Parser parser;
    Scope global_scope = {"Global", NULL, NULL};
    
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(input, sizeof(input), stdin)) break;
        if (strcmp(input, "exit\n") == 0 || strcmp(input, "quit\n") == 0) {
            break;
        }
        
        lexer = (Lexer){input, 0, input[0]};
        parser = (Parser){&lexer, lexer_next(&lexer)};

        Ast* tree = parse_expr(&parser);
        if (!tree) {
            printf("Error: Failed to parse expression.\n");
            continue;
        }
        
        Value result = eval(tree, &global_scope);
        if (result.type == VAL_NUMBER) {
            printf("= %g\n", result.value.number);
        } else {
            printf("= <non-number result>\n");
        }

        ast_free(tree);
    }

    return 0;
}
#include "stdio.h"

#include "ast.h"
#include "eval.h"
#include "token.h"
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

    while (1)
    {
        printf("%s", prompt);
        scanf("%s", input);
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }
        
        lexer = (Lexer){input, 0, input[0]};
        parser = (Parser){&lexer, lexer_next(&lexer)};
        Ast* ast = parse_expr(&parser);
        double result = eval(ast);
        printf("%g\n", result);
    }

    return 0;
}
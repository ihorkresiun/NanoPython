#include "stdio.h"

#include "eval.h"
#include "token.h"
#include "parser.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s \"expression\"\n", prog_name);
    printf("Evaluates the arithmetic expression provided as a command-line argument.\n");
    printf("Example: %s \"3 + 4 * (2 - 1)\"\n", prog_name);
}   

int main(int argc, char** argv)
{
    if(argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input = argv[1];
    printf("Input: %s\n", input);


    Lexer lexer = {input, 0, input[0]};
    Parser parser = {&lexer, lexer_next(&lexer)};
    Expr* ast = parse_expr(&parser);
    double result = eval(ast);
    printf("%g\n", result);

    return 0;
}
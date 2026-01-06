#include "stdio.h"

#include "eval.h"
#include "token.h"
#include "parser.h"

int main(int argc, char** argv)
{
    // TODO print help

    const char* input = argv[1];
    Lexer lexer = {input, 0, input[0]};
    Parser parser = {&lexer, lexer_next(&lexer)};

    Expr* ast = parse_expr(&parser);
    double result = eval(ast);
    printf("%g\n", result);

    return 0;
}
#ifndef __INC_PARSER_H__
#define __INC_PARSER_H__

#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

void parser_init(Parser* p, const char* input);
Ast* parse_statement(Parser* p);
Ast* parse_program(Parser* p);
#endif
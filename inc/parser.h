#ifndef __INC_PARSER_H__
#define __INC_PARSER_H__

#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;


Ast * parse_expr(Parser* p);

#endif
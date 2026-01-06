#ifndef __INC_PARSER_H__
#define __INC_PARSER_H__

#include "ast.h"
#include "token.h"

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;


Ast * parse_expr(Parser* p);

#endif
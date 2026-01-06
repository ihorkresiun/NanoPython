#ifndef __INC_PARSER_H__
#define __INC_PARSER_H__

#include "token.h"

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;


Expr* parse_expr(Parser* p);

#endif
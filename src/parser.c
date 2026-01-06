#include "ast.h"
#include "parser.h"
#include "lexer.h"

#include "stdlib.h"
#include "stdio.h"


void parser_init(Parser* p, const char* input) {
    p->lexer->input = input;
    p->lexer->pos = 0;
    p->current = lexer_next(p->lexer);
}

void parser_eat(Parser* p, TokenType type) {
    if (p->current.type == type) {
        p->current = lexer_next(p->lexer);
    } else {
        printf("Unexpected token\n");
        exit(1);
    }
}

Ast* parse_factor(Parser* p) {
    Token tok = p->current;

    if (tok.type == TOKEN_NUMBER) {
        parser_eat(p, TOKEN_NUMBER);
        return ast_new_number(tok.value);
    }

    if (tok.type == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        Ast* node = parse_expr(p);
        parser_eat(p, TOKEN_RPAREN);
        return node;
    }

    printf("Invalid factor\n");
    exit(1);
}

Ast* parse_term(Parser* p) {
    Ast* node = parse_factor(p);

    while (p->current.type == TOKEN_STAR || p->current.type == TOKEN_SLASH || p->current.type == TOKEN_CARET) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_factor(p));
    }

    return node;
}

Ast* parse_expr(Parser* p) {
    Ast* node = parse_term(p);

    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_term(p));
    }

    return node;
}
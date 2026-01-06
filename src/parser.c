#include "parser.h"
#include "token.h"

#include "stdlib.h"
#include "stdio.h"


void parser_eat(Parser* p, TokenType type) {
    if (p->current.type == type) {
        p->current = lexer_next(p->lexer);
    } else {
        printf("Unexpected token\n");
        exit(1);
    }
}

Expr* new_number(double value) {
    Expr* e = malloc(sizeof(Expr));
    e->type = EXPR_NUMBER;
    e->value = value;
    e->left = e->right = NULL;
    return e;
}

Expr* new_expr(ExprType type, Expr* left, Expr* right) {
    Expr* e = malloc(sizeof(Expr));
    e->type = type;
    e->left = left;
    e->right = right;
    return e;
}

Expr* parse_factor(Parser* p) {
    if (p->current.type == TOKEN_NUMBER) {
        double val = p->current.value;
        parser_eat(p, TOKEN_NUMBER);
        return new_number(val);
    } else if (p->current.type == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        Expr* node = parse_expr(p);
        parser_eat(p, TOKEN_RPAREN);
        return node;
    } else {
        printf("Unexpected token in factor\n");
        exit(1);
    }
}

Expr* parse_term(Parser* p) {
    Expr* node = parse_factor(p);
    while (p->current.type == TOKEN_STAR || p->current.type == TOKEN_SLASH) {
        TokenType op = p->current.type;
        if (op == TOKEN_STAR) {
            parser_eat(p, TOKEN_STAR);
            node = new_expr(EXPR_MUL, node, parse_factor(p));
        } else {
            parser_eat(p, TOKEN_SLASH);
            node = new_expr(EXPR_DIV, node, parse_factor(p));
        }
    }
    return node;
}

Expr* parse_expr(Parser* p)
{
    Expr* node = parse_term(p);
    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType op = p->current.type;
        if (op == TOKEN_PLUS) {
            parser_eat(p, TOKEN_PLUS);
            node = new_expr(EXPR_ADD, node, parse_term(p));
        } else {
            parser_eat(p, TOKEN_MINUS);
            node = new_expr(EXPR_SUB, node, parse_term(p));
        }
    }
    return node;
}

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "vars.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// Forward declaration of lower-level parsing functions
Ast* parse_logic_or(Parser* p);

void parser_init(Parser* p, const char* input) {
    p->lexer->input = input;
    p->lexer->pos = 0;
    p->current = lexer_next(p->lexer);
}

void parser_eat(Parser* p, TokenType type) {
    if (p->current.type == type) {
        p->current = lexer_next(p->lexer);
    } else {
        printf("Unexpected token: expected %d, got %d\n", type, p->current.type);
        exit(1);
    }
}

Ast* parse_factor(Parser* p) {
    Token tok = p->current;

    if (tok.type == TOKEN_NUMBER) {
        parser_eat(p, TOKEN_NUMBER);
        return ast_new_number(tok.value.value.number);
    }

    if (tok.type == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        Ast* node = parse_logic_or(p);
        parser_eat(p, TOKEN_RPAREN);
        return node;
    }

    printf("Invalid factor\n");
    exit(1);
}

Ast* parse_unary(Parser* p) {
    Token tok = p->current;

    if (tok.type == TOKEN_MINUS) {
        parser_eat(p, TOKEN_MINUS);
        Ast* node = parse_unary(p);
        return ast_new_unary(TOKEN_MINUS, node);
    }

    if (tok.type == TOKEN_NOT) {
        parser_eat(p, TOKEN_NOT);
        Ast* node = parse_unary(p);
        return ast_new_unary(TOKEN_NOT, node);
    }

    return parse_factor(p);
}

Ast* parse_term(Parser* p) {
    Ast* node = parse_unary(p);

    while (p->current.type == TOKEN_STAR || p->current.type == TOKEN_SLASH || p->current.type == TOKEN_CARET) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_factor(p));
    }

    return node;
}

Ast* parse_arithmetic(Parser* p) {
    Ast* node = parse_term(p);

    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_term(p));
    }

    return node;
}

Ast* parse_comparison(Parser* p) {
    Ast* left = parse_arithmetic(p);
    
    TokenType op = p->current.type;
    if (op == TOKEN_LT || op == TOKEN_GT || op == TOKEN_LE ||
        op == TOKEN_GE || op == TOKEN_EQ || op == TOKEN_NE) {
        parser_eat(p, op);
        Ast* right = parse_arithmetic(p);
        left = ast_new_expr(op, left, right);
    }

    return left;
}

Ast* parse_logic_and(Parser* p) {
    Ast* left = parse_comparison(p);

    while (p->current.type == TOKEN_AND) {
        parser_eat(p, TOKEN_AND);
        Ast* right = parse_comparison(p);
        left = ast_new_expr(TOKEN_AND, left, right);
    }

    return left;
}

Ast* parse_logic_or(Parser* p) {
    Ast* left = parse_logic_and(p);

    while (p->current.type == TOKEN_OR) {
        parser_eat(p, TOKEN_OR);
        Ast* right = parse_logic_and(p);
        left = ast_new_expr(TOKEN_OR, left, right);
    }

    return left;
}

Ast* parse_block(Parser* p) {
    Ast** stmts = NULL;
    int count = 0;

    while (p->current.type != TOKEN_DEDENT && p->current.type != TOKEN_EOF) {
        if (p->current.type == TOKEN_NEWLINE) {
            parser_eat(p, TOKEN_NEWLINE);
            continue;
        }
        Ast* stmt = parse_statement(p);
        stmts = realloc(stmts, sizeof(Ast*) * (count + 1));
        stmts[count++] = stmt;
    }

    if (p->current.type == TOKEN_DEDENT) {
        parser_eat(p, TOKEN_DEDENT);
    }

    return ast_new_block(stmts, count);
}

Ast* parse_if(Parser* p) {
    parser_eat(p, TOKEN_IF);
    Ast* condition = parse_logic_or(p);
    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    Ast* then_block = parse_block(p);

    Ast* else_block = NULL;
    if (p->current.type == TOKEN_ELSE) {
        parser_eat(p, TOKEN_ELSE);
        parser_eat(p, TOKEN_COLON);
        parser_eat(p, TOKEN_NEWLINE);
        parser_eat(p, TOKEN_INDENT);
        else_block = parse_block(p);
    }

    return ast_new_if(condition, then_block, else_block);
}

Ast* parse_while(Parser* p) {
    parser_eat(p, TOKEN_WHILE);
    Ast* condition = parse_logic_or(p);
    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    Ast* body = parse_block(p);

    return ast_new_while(condition, body);
}

Ast* parse_def(Parser* p) {

}

Ast* parse_call(Parser* p, const char* func_name) {

}

Ast* parse_assignment(Parser* p, const char* var_name) {
    Token tok = p->current;
    if (tok.type == TOKEN_IDENT) {
        parser_eat(p, TOKEN_IDENT);

        if (p->current.type == TOKEN_ASSIGN) {
            parser_eat(p, TOKEN_ASSIGN);
            Ast* value = parse_logic_or(p);

            return ast_new_assign(tok.ident, value);
        }
        
        return ast_new_var(tok.ident);
    }
}

Ast* parse_print(Parser* p) {
    parser_eat(p, TOKEN_PRINT);
    parser_eat(p, TOKEN_LPAREN);
    Ast* expr = parse_logic_or(p);
    parser_eat(p, TOKEN_RPAREN);
    return ast_new_print(expr);
}

Ast* parse_statement(Parser* p) {
    switch(p->current.type) {
        case TOKEN_IF:
            return parse_if(p);
        case TOKEN_WHILE:
            return parse_while(p);
        case TOKEN_DEF:
            return parse_def(p);
        case TOKEN_IDENT:
            return parse_assignment(p, p->current.ident);
        case TOKEN_PRINT:
            return parse_print(p);
        default:
            // Start parsing an expression
            // parse_logic_or -> 
            // parse_logic_and ->
            // parse_comparison ->
            // parse_arithmetic ->
            // parse_term ->
            // parse_unary ->
            // parse_factor
            return parse_logic_or(p);
    }
}
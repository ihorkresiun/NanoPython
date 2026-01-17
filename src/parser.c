#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "vars.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// Forward declaration of lower-level parsing functions
static Ast* parse_logic_or(Parser* p);
static Ast* parse_call(Parser* p, const char* func_name);

void parser_init(Parser* p, const char* input) {
    p->lexer->input = input;
    p->lexer->pos = 0;
    p->current = lexer_next(p->lexer);
}

static void parser_eat(Parser* p, TokenType type) {
    if (p->current.type == type) {
        p->current = lexer_next(p->lexer);
    } else {
         printf("Unexpected token: expected %d, got %d\n", type, p->current.type);
        exit(1);
    }
}

static Ast* parse_factor(Parser* p) {
    Token tok = p->current;

    if (tok.type == TOKEN_NUMBER) {
        parser_eat(p, TOKEN_NUMBER);
        return ast_new_number(tok.value.value.number);
    }

    if (tok.type == TOKEN_STRING) {
        parser_eat(p, TOKEN_STRING);
        return ast_new_string(tok.value.value.string);
    }

    if (tok.type == TOKEN_IDENT) {
        parser_eat(p, TOKEN_IDENT);

        if (p->current.type == TOKEN_LPAREN) {
            return parse_call(p, tok.ident);
        }

        return ast_new_var(tok.ident);
    }

    if (tok.type == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        Ast* node = parse_logic_or(p);
        parser_eat(p, TOKEN_RPAREN);
        return node;
    }

    printf("Invalid factor %d\n", tok.type);
    exit(1);
}

static Ast* parse_unary(Parser* p) {
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

static Ast* parse_term(Parser* p) {
    Ast* node = parse_unary(p);

    while (p->current.type == TOKEN_STAR || p->current.type == TOKEN_SLASH || p->current.type == TOKEN_CARET) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_factor(p));
    }

    return node;
}

static Ast* parse_arithmetic(Parser* p) {
    Ast* node = parse_term(p);

    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        TokenType op = p->current.type;
        parser_eat(p, op);
        node = ast_new_expr(op, node, parse_term(p));
    }

    return node;
}

static Ast* parse_comparison(Parser* p) {
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

static Ast* parse_logic_and(Parser* p) {
    Ast* left = parse_comparison(p);

    while (p->current.type == TOKEN_AND) {
        parser_eat(p, TOKEN_AND);
        Ast* right = parse_comparison(p);
        left = ast_new_expr(TOKEN_AND, left, right);
    }

    return left;
}

static Ast* parse_logic_or(Parser* p) {
    Ast* left = parse_logic_and(p);

    while (p->current.type == TOKEN_OR) {
        parser_eat(p, TOKEN_OR);
        Ast* right = parse_logic_and(p);
        left = ast_new_expr(TOKEN_OR, left, right);
    }

    return left;
}

static Ast* parse_block(Parser* p) {
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

static Ast* parse_if(Parser* p) {
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

static Ast* parse_while(Parser* p) {
    parser_eat(p, TOKEN_WHILE);
    Ast* condition = parse_logic_or(p);
    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    Ast* body = parse_block(p);

    return ast_new_while(condition, body);
}

static Ast* parse_def(Parser* p) {
    parser_eat(p, TOKEN_DEF);

    if (p->current.type != TOKEN_IDENT) {
        printf("Expected function name after 'def'\n");
        exit(1);
    }

    const char* func_name = p->current.ident;
    parser_eat(p, TOKEN_IDENT);

    parser_eat(p, TOKEN_LPAREN);
    char** args = NULL;
    int argc = 0;

    if (p->current.type != TOKEN_RPAREN) {
        while (1) {
            if (p->current.type != TOKEN_IDENT) {
                printf("Expected parameter name in function definition\n");
                exit(1);
            }
            args = realloc(args, sizeof(char*) * (argc + 1));
            args[argc++] = strdup(p->current.ident);
            parser_eat(p, TOKEN_IDENT);

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);
            } else {
                break;
            }
        }
    }

    parser_eat(p, TOKEN_RPAREN);
    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    Ast* body = parse_block(p);

    return ast_new_funcdef(func_name, args, argc, body);
}

static Ast* parse_call(Parser* p, const char* func_name) {
    parser_eat(p, TOKEN_LPAREN);

    Ast** args = NULL;
    int argc = 0;

    if (p->current.type != TOKEN_RPAREN) {
        while (1) {
            Ast* arg = parse_logic_or(p);
            args = realloc(args, sizeof(Ast*) * (argc + 1));
            args[argc++] = arg;

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);
            } else {
                break;
            }
        }
    }
    parser_eat(p, TOKEN_RPAREN);
    return ast_new_call(func_name, args, argc);
}

static int tocken_endline_or_eof(TokenType type) {
    return type == TOKEN_NEWLINE || type == TOKEN_EOF || type == TOKEN_DEDENT;
}

static Ast* parse_return(Parser* p) {
    parser_eat(p, TOKEN_RETURN);

    if (tocken_endline_or_eof(p->current.type)) {
        return ast_new_return(NULL);
    }

    Ast* value = parse_logic_or(p);
    return ast_new_return(value);
}

static Ast* parse_break(Parser* p) {
    parser_eat(p, TOKEN_BREAK);
    return ast_new_break();
}

static Ast* parse_continue(Parser* p) {
    parser_eat(p, TOKEN_CONTINUE);
    return ast_new_continue();
}

static Ast* parse_assignment(Parser* p) {
    Token tok = p->current;
    if (tok.type == TOKEN_IDENT) {
        Token next_token = lexer_peek_next(p->lexer);

        if (next_token.type == TOKEN_ASSIGN) {
            // a = 2 + 3
            // Eat identifier 'a'
            parser_eat(p, TOKEN_IDENT);
            // Eat '='
            parser_eat(p, TOKEN_ASSIGN);
            // Parse the expression on the right side
            Ast* value = parse_logic_or(p);
            return ast_new_assign(tok.ident, value);
        } else if (tocken_endline_or_eof(next_token.type)) {
            // Just a variable reference
            return ast_new_var(tok.ident);
        }
        
        // Not an assignment, parse as expression, a + b, a + 1 etc.
        return parse_logic_or(p);
    }
}

static Ast* parse_print(Parser* p) {
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
        case TOKEN_RETURN:
            return parse_return(p);
        case TOKEN_BREAK:
            return parse_break(p);
        case TOKEN_CONTINUE:
            return parse_continue(p);
        case TOKEN_IDENT:
            // Identifier or assignment
            return parse_assignment(p);
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
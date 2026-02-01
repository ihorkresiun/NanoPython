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
    p->next = lexer_next(p->lexer);
}

static void parser_eat(Parser* p, TokenType type) {
    if (p->current.type == type) {
        p->current = p->next;
        p->next = lexer_next(p->lexer);
    } else {
        printf("Unexpected token: expected %d, got %d\n", type, p->current.type);
        exit(1);
    }
}

static Ast* parse_factor(Parser* p) {
    TokenType op = p->current.type;

    if (op == TOKEN_NUMBER) {
        Ast* ret = ast_new_number(p->current.value.as.integer);
        parser_eat(p, TOKEN_NUMBER);
        return ret;
    }

    if (op == TOKEN_FLOAT) {
        Ast* ret = ast_new_number_float(p->current.value.as.floating);
        parser_eat(p, TOKEN_FLOAT);
        return ret;
    }

    if (op == TOKEN_STRING) {
        Ast* ret = ast_new_string(as_string(p->current.value)->chars);
        parser_eat(p, TOKEN_STRING);
        return ret;
    }

    if (op == TOKEN_IDENT) {
        const char* ident_name = strdup(p->current.ident);
        parser_eat(p, TOKEN_IDENT);
        if (p->current.type == TOKEN_LPAREN) {
            return parse_call(p, ident_name);
        }

        if (p->current.type == TOKEN_LBRACKET) {
            parser_eat(p, TOKEN_LBRACKET);
            Ast* index = parse_logic_or(p);
            parser_eat(p, TOKEN_RBRACKET);

            if (p->current.type != TOKEN_ASSIGN) {
                // Just a list index access a[0]
                Ast* target = ast_new_var(ident_name);
                return ast_new_index(target, index);
            }

            parser_eat(p, TOKEN_ASSIGN);
            // Assignment to list index a[0] = 5
            Ast* value = parse_logic_or(p);
            Ast* target = ast_new_var(ident_name);
            return ast_new_assign_index(target, index, value);
        }

        return ast_new_var(ident_name);
    }

    if (op == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        Ast* node = parse_logic_or(p);
        parser_eat(p, TOKEN_RPAREN);
        return node;
    }

    if (op == TOKEN_LBRACKET) {
        parser_eat(p, TOKEN_LBRACKET);
        
        Ast** elements = NULL; 
        int count = 0;

        while (p->current.type != TOKEN_RBRACKET) {
            Ast* elem = parse_logic_or(p);
            elements = realloc(elements, sizeof(Ast*) * (count + 1));
            elements[count++] = elem;

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);
            } else {
                break;
            }
        }
    
        parser_eat(p, TOKEN_RBRACKET);
        return ast_new_list(elements, count);
    }
    printf("Unknown factor %d\n", op);
    return NULL;
}

static Ast* parse_unary(Parser* p) {
    Token tok = p->current;

    if (tok.type == TOKEN_MINUS) {
        parser_eat(p, TOKEN_MINUS);
        Ast* node = parse_unary(p);
        return ast_new_unary_expr(TOKEN_MINUS, node);
    }

    if (tok.type == TOKEN_NOT) {
        parser_eat(p, TOKEN_NOT);
        Ast* node = parse_unary(p);
        return ast_new_unary_expr(TOKEN_NOT, node);
    }

    return parse_factor(p);
}

static Ast* parse_term(Parser* p) {
    Ast* node = parse_unary(p);

    TokenType op = p->current.type;
    while (op == TOKEN_STAR || op == TOKEN_SLASH || op == TOKEN_CARET) {
        parser_eat(p, op);
        node = ast_new_binary_expr(op, node, parse_factor(p));
        op = p->current.type;
    }

    return node;
}

static Ast* parse_arithmetic(Parser* p) {
    Ast* node = parse_term(p);

    TokenType op = p->current.type;
    while (op == TOKEN_PLUS || op == TOKEN_MINUS) {
        parser_eat(p, op);
        node = ast_new_binary_expr(op, node, parse_term(p));
        op = p->current.type;
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
        left = ast_new_binary_expr(op, left, right);
    }

    return left;
}

static Ast* parse_logic_and(Parser* p) {
    Ast* left = parse_comparison(p);

    TokenType op = p->current.type;
    while (op == TOKEN_AND) {
        parser_eat(p, TOKEN_AND);
        Ast* right = parse_comparison(p);
        left = ast_new_binary_expr(TOKEN_AND, left, right);
        op = p->current.type;
    }

    return left;
}

static Ast* parse_logic_or(Parser* p) {
    Ast* left = parse_logic_and(p);

    TokenType op = p->current.type;
    while (op == TOKEN_OR) {
        parser_eat(p, TOKEN_OR);
        Ast* right = parse_logic_and(p);
        left = ast_new_binary_expr(TOKEN_OR, left, right);
        op = p->current.type;
    }

    return left;
}

static Ast* parse_block(Parser* p) {
    Ast** stmts = NULL;
    int count = 0;

    TokenType op = p->current.type;
    while (op != TOKEN_DEDENT && op != TOKEN_EOF) {
        if (p->current.type == TOKEN_NEWLINE) {
            parser_eat(p, TOKEN_NEWLINE);
            op = p->current.type;
            continue;
        }
        Ast* stmt = parse_statement(p);
        stmts = realloc(stmts, sizeof(Ast*) * (count + 1));
        stmts[count++] = stmt;
        op = p->current.type;
    }

    parser_eat(p, TOKEN_DEDENT);

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

static Ast* parse_for(Parser* p) {
    parser_eat(p, TOKEN_FOR);

    if (p->current.type != TOKEN_IDENT) {
        printf("Expected iterator variable name after 'for'\n");
        exit(1);
    }

    const char* var_name = strdup(p->current.ident);

    parser_eat(p, TOKEN_IDENT);
    parser_eat(p, TOKEN_IN);

    Ast* iterable = parse_logic_or(p);

    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    Ast* body = parse_block(p);

    return ast_new_for(var_name, iterable, body);
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

static int token_endline_or_eof(TokenType type) {
    return type == TOKEN_NEWLINE || type == TOKEN_EOF || type == TOKEN_DEDENT;
}

static Ast* parse_return(Parser* p) {
    parser_eat(p, TOKEN_RETURN);

    if (token_endline_or_eof(p->current.type)) {
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

static Ast* parse_ident(Parser* p) {
    Token tok = p->current;
    if (tok.type == TOKEN_IDENT) {

        if (token_endline_or_eof(p->next.type)) {
            // Just a variable reference
            return ast_new_var(tok.ident);
        }

        if (p->next.type == TOKEN_ASSIGN) {
            // Assignment to variable
            parser_eat(p, TOKEN_IDENT);
            parser_eat(p, TOKEN_ASSIGN);

            // Parse the expression on the right side
            Ast* value = parse_logic_or(p);
            return ast_new_assign(tok.ident, value);
        }
        
        // Not an assignment, parse as expression, a + b, a + 1 etc.
        return parse_logic_or(p);
    }
}

Ast* parse_statement(Parser* p) {
    switch(p->current.type) {
        case TOKEN_IF:
            return parse_if(p);
        case TOKEN_WHILE:
            return parse_while(p);
        case TOKEN_FOR:
            return parse_for(p);
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
            return parse_ident(p);
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

Ast* parse_program(Parser* p)
{
    Ast** stmts = NULL;
    int count = 0;

    while (p->current.type != TOKEN_EOF) {
        if (p->current.type == TOKEN_NEWLINE) {
            parser_eat(p, TOKEN_NEWLINE);
            continue;
        }

        Ast* stmt = parse_statement(p);
        stmts = realloc(stmts, sizeof(Ast*) * (count + 1));
        stmts[count++] = stmt;
    }

    return ast_new_block(stmts, count);
}
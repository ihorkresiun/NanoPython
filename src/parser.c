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
        printf("Unexpected token: expected %d, got %d, at line %d, column %d\n", type, p->current.type, p->current.line, p->current.col);
        exit(1);
    }
}

static void parser_eat_newlines(Parser* p) {
    while (p->current.type == TOKEN_NEWLINE) {
        parser_eat(p, TOKEN_NEWLINE);
    }
}

static void parser_eat_indents(Parser* p) {
    while (p->current.type == TOKEN_INDENT) {
        parser_eat(p, TOKEN_INDENT);
    }
}

static void parser_eat_dedents(Parser* p) {
    while (p->current.type == TOKEN_DEDENT) {
        parser_eat(p, TOKEN_DEDENT);
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
        
        Ast* base = ast_new_var(ident_name);
        
        // Handle dot notation (obj.attr or obj.method())
        while (p->current.type == TOKEN_DOT) {
            parser_eat(p, TOKEN_DOT);
            
            if (p->current.type != TOKEN_IDENT) {
                printf("Expected attribute/method name after '.', at line %d, column %d\n", p->current.line, p->current.col);
                exit(1);
            }
            
            const char* attr_name = strdup(p->current.ident);
            parser_eat(p, TOKEN_IDENT);
            
            if (p->current.type == TOKEN_LPAREN) {
                // Method call obj.method(args)
                parser_eat(p, TOKEN_LPAREN);
                
                Ast** args = NULL;
                int argc = 0;
                
                while (p->current.type != TOKEN_RPAREN) {
                    parser_eat_newlines(p);
                    parser_eat_indents(p);
                    Ast* arg = parse_logic_or(p);
                    args = realloc(args, sizeof(Ast*) * (argc + 1));
                    args[argc++] = arg;
                    
                    if (p->current.type == TOKEN_COMMA) {
                        parser_eat(p, TOKEN_COMMA);
                    } else {
                        parser_eat_newlines(p);
                        parser_eat_dedents(p);
                    }
                }
                parser_eat(p, TOKEN_RPAREN);
                base = ast_new_method_call(base, attr_name, args, argc);
            } else if (p->current.type == TOKEN_ASSIGN) {
                // Attribute assignment obj.attr = value
                parser_eat(p, TOKEN_ASSIGN);
                Ast* value = parse_logic_or(p);
                return ast_new_attr_assign(base, attr_name, value);
            } else {
                // Attribute access obj.attr
                base = ast_new_attr_access(base, attr_name);
            }
        }
        
        // After processing all dots, check for function call or indexing
        // But only if we haven't already processed a method call
        if (base->type == AST_VAR && p->current.type == TOKEN_LPAREN) {
            // Regular function call (no dots were involved)
            return parse_call(p, ident_name);
        }

        if (p->current.type == TOKEN_LBRACKET) {
            parser_eat(p, TOKEN_LBRACKET);

            parser_eat_newlines(p);
            parser_eat_indents(p);
            
            Ast* index = parse_logic_or(p);

            parser_eat_newlines(p);
            parser_eat_dedents(p);

            parser_eat(p, TOKEN_RBRACKET);

            if (p->current.type != TOKEN_ASSIGN) {
                // Just a list index access a[0]
                return ast_new_index(base, index);
            }

            parser_eat(p, TOKEN_ASSIGN);
            // Assignment to list index a[0] = 5
            Ast* value = parse_logic_or(p);
            return ast_new_assign_index(base, index, value);
        }

        return base;
    }

    if (op == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);

        parser_eat_newlines(p);
        parser_eat_indents(p);
        
        // Empty tuple ()
        if (p->current.type == TOKEN_RPAREN) {
            parser_eat(p, TOKEN_RPAREN);
            return ast_new_tuple(NULL, 0);
        }
        
        // Parse first element
        Ast* first = parse_logic_or(p);
        
        // Check if it's a tuple (has comma) or just parenthesized expression
        if (p->current.type == TOKEN_COMMA) {
            // It's a tuple
            Ast** elements = malloc(sizeof(Ast*));
            elements[0] = first;
            int count = 1;
            
            parser_eat(p, TOKEN_COMMA);
            
            while (p->current.type != TOKEN_RPAREN) {
                // Skip newlines and indents between elements
                parser_eat_newlines(p);
                parser_eat_indents(p);
                
                Ast* elem = parse_logic_or(p);
                elements = realloc(elements, sizeof(Ast*) * (count + 1));
                elements[count++] = elem;

                if (p->current.type == TOKEN_COMMA) {
                    parser_eat(p, TOKEN_COMMA);
                } else {
                    parser_eat_newlines(p);
                    parser_eat_dedents(p);
                    break;
                }
            }
            
            parser_eat(p, TOKEN_RPAREN);
            return ast_new_tuple(elements, count);
        } else {
            // Just a parenthesized expression
            parser_eat(p, TOKEN_RPAREN);
            return first;
        }
    }

    if (op == TOKEN_LBRACKET) {
        // List literal
        parser_eat(p, TOKEN_LBRACKET);
        
        Ast** elements = NULL; 
        int count = 0;

        while (p->current.type != TOKEN_RBRACKET) {
            // Skip newlines and indents between elements
            parser_eat_newlines(p);
            parser_eat_indents(p);
            
            Ast* elem = parse_logic_or(p);
            elements = realloc(elements, sizeof(Ast*) * (count + 1));
            elements[count++] = elem;

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);
            } else {
                parser_eat_newlines(p);
                parser_eat_dedents(p);
                break;
            }
        }
    
        parser_eat(p, TOKEN_RBRACKET);
        return ast_new_list(elements, count);
    }

    if (op == TOKEN_LBRACE) {
        // Dictionary or Set literal
        parser_eat(p, TOKEN_LBRACE);
        // Skip newlines and indents before first element
        parser_eat_newlines(p);
        parser_eat_indents(p);
        
        // Empty dict {}
        if (p->current.type == TOKEN_RBRACE) {
            parser_eat(p, TOKEN_RBRACE);
            return ast_new_dict(NULL, NULL, 0);
        }
        
        // Parse first element to determine if it's a dict or set
        Ast* first = parse_logic_or(p);
        
        if (p->current.type == TOKEN_COLON) {
            // It's a dictionary {key: value, ...}
            parser_eat(p, TOKEN_COLON);
            Ast* first_value = parse_logic_or(p);
            
            Ast** keys = malloc(sizeof(Ast*));
            Ast** values = malloc(sizeof(Ast*));
            keys[0] = first;
            values[0] = first_value;
            int count = 1;

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);
                while (p->current.type != TOKEN_RBRACE) {
                    // Skip newlines and indents between elements
                    parser_eat_newlines(p);
                    parser_eat_indents(p);

                    Ast* key = parse_logic_or(p);
                    parser_eat(p, TOKEN_COLON);
                    Ast* value = parse_logic_or(p);

                    keys = realloc(keys, sizeof(Ast*) * (count + 1));
                    values = realloc(values, sizeof(Ast*) * (count + 1));
                    keys[count] = key;
                    values[count] = value;
                    count++;

                    if (p->current.type == TOKEN_COMMA) {
                        parser_eat(p, TOKEN_COMMA);
                    } else {
                        parser_eat_newlines(p);
                        parser_eat_dedents(p);
                        break;
                    }
                }
            }
        
            parser_eat(p, TOKEN_RBRACE);
            return ast_new_dict(keys, values, count);
        } else {
            // It's a set {elem1, elem2, ...}
            Ast** elements = malloc(sizeof(Ast*));
            elements[0] = first;
            int count = 1;

            if (p->current.type == TOKEN_COMMA) {
                parser_eat(p, TOKEN_COMMA);

                while (p->current.type != TOKEN_RBRACE) {
                    parser_eat_newlines(p);
                    parser_eat_dedents(p);
                    Ast* elem = parse_logic_or(p);
                    elements = realloc(elements, sizeof(Ast*) * (count + 1));
                    elements[count++] = elem;

                    if (p->current.type == TOKEN_COMMA) {
                        parser_eat(p, TOKEN_COMMA);
                    } else {
                        parser_eat_newlines(p);
                        parser_eat_dedents(p);
                        break;
                    }
                }
            }
        
            parser_eat(p, TOKEN_RBRACE);
            return ast_new_set(elements, count);
        }
    }

    printf("Unknown factor %d, line %d, column %d\n", op, p->current.line, p->current.col);
    exit(1);
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
    while (op != TOKEN_DEDENT && op != TOKEN_EOF && op != TOKEN_ELSE) {
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

    // Only eat DEDENT if we stopped because of DEDENT (not ELSE or EOF)
    if (op == TOKEN_DEDENT) {
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

static Ast* parse_class(Parser* p) {
    parser_eat(p, TOKEN_CLASS);

    if (p->current.type != TOKEN_IDENT) {
        printf("Expected class name after 'class'\n");
        exit(1);
    }

    const char* class_name = strdup(p->current.ident);
    parser_eat(p, TOKEN_IDENT);

    // Optional parent class
    char* parent_name = NULL;
    if (p->current.type == TOKEN_LPAREN) {
        parser_eat(p, TOKEN_LPAREN);
        if (p->current.type == TOKEN_IDENT) {
            parent_name = strdup(p->current.ident);
            parser_eat(p, TOKEN_IDENT);
        }
        parser_eat(p, TOKEN_RPAREN);
    }

    parser_eat(p, TOKEN_COLON);
    parser_eat(p, TOKEN_NEWLINE);
    parser_eat(p, TOKEN_INDENT);

    // Parse methods
    Ast** methods = NULL;
    int method_count = 0;

    while (p->current.type != TOKEN_DEDENT && p->current.type != TOKEN_EOF) {
        if (p->current.type == TOKEN_NEWLINE) {
            parser_eat(p, TOKEN_NEWLINE);
            continue;
        }

        if (p->current.type == TOKEN_DEF) {
            Ast* method = parse_def(p);
            methods = realloc(methods, sizeof(Ast*) * (method_count + 1));
            methods[method_count++] = method;
        } else {
            printf("Expected method definition in class body\n");
            exit(1);
        }

        if (p->current.type == TOKEN_NEWLINE) {
            parser_eat(p, TOKEN_NEWLINE);
        }
    }

    if (p->current.type != TOKEN_EOF) {
        parser_eat(p, TOKEN_DEDENT);
    }

    return ast_new_classdef(class_name, parent_name, methods, method_count);
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

static Ast* parse_import(Parser* p) {
    parser_eat(p, TOKEN_IMPORT);
    
    if (p->current.type != TOKEN_IDENT) {
        printf("Expected module name after 'import'\n");
        exit(1);
    }
    
    char* module_name = strdup(p->current.ident);
    parser_eat(p, TOKEN_IDENT);
    
    return ast_new_import(module_name);
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
        case TOKEN_CLASS:
            return parse_class(p);
        case TOKEN_RETURN:
            return parse_return(p);
        case TOKEN_BREAK:
            return parse_break(p);
        case TOKEN_CONTINUE:
            return parse_continue(p);
        case TOKEN_IMPORT:
            return parse_import(p);
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
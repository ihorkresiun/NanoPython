#include "lexer.h"
#include "vars.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

static int indent_stack[64] = {0};
static int indent_top = 0;

TokenType handle_new_line(Lexer* lexer) {
    size_t start_pos = lexer->pos;

    // Count spaces
    int space_count = 0;
    while (lexer->input[lexer->pos] == ' ') {
        space_count++;
        lexer->pos++;
    }

    if (lexer->input[lexer->pos] == '\n' || lexer->input[lexer->pos] == '\0') {
        // Empty line, ignore
        return TOKEN_EMPTY;
    }

    if (space_count > indent_stack[indent_top]) {
        // Indent
        indent_stack[++indent_top] = space_count;
        return TOKEN_INDENT;
    } else {
        while (space_count < indent_stack[indent_top]) {
            // Dedent
            indent_top--;
            return TOKEN_DEDENT;
        }
    }
    return TOKEN_NEWLINE ;
}

Token lexer_next(Lexer * lexer)
{
    // Skip whitespace
    while (isspace(lexer->input[lexer->pos])) lexer->pos++;

    // Todo: keywords dict
    // char* keywords[] = {"if", "else", "while", "def", NULL};
    const char* input = lexer->input + lexer->pos;
    if (strcmp(input, "if") == 0) return (Token){TOKEN_IF, make_none()};
    if (strcmp(input, "else") == 0) return (Token){TOKEN_ELSE, make_none()};
    if (strcmp(input, "while") == 0) return (Token){TOKEN_WHILE, make_none()};
    if (strcmp(input, "def") == 0) return (Token){TOKEN_FUNCDEF, make_none()};
    const char c = lexer->input[lexer->pos];

    if (c == '\0') return (Token){TOKEN_EOF, make_none()};

    if(isdigit(c) || c == '.') {
        // Parse number
        char buffer[64];
        size_t i = 0;
        while(isdigit(lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '.') {
            buffer[i++] = lexer->input[lexer->pos++];
        }
        buffer[i] = '\0';
        Token tok;
        tok.type = TOKEN_NUMBER;
        tok.value = make_number(atof(buffer));
        return tok;
    }

    if (isalpha(c) || c == '_') {
        char buf[64];
        size_t i = 0;

        while (isalnum(lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '_') {
            buf[i++] = lexer->input[lexer->pos++];
        }
        buf[i] = '\0';

        Token tok;
        tok.type = TOKEN_IDENT;
        tok.ident = strdup(buf);   // важливо: malloc!
        return tok;
    }

    lexer->pos++; // advance for single char tokens

    switch(c) {
        case '+': return (Token){TOKEN_PLUS, make_none()};
        case '-': return (Token){TOKEN_MINUS, make_none()};
        case '*': return (Token){TOKEN_STAR, make_none()};
        case '/': return (Token){TOKEN_SLASH, make_none()};
        case '(': return (Token){TOKEN_LPAREN, make_none()};
        case ')': return (Token){TOKEN_RPAREN, make_none()};
        case '^': return (Token){TOKEN_CARET, make_none()};
        case '=': return (Token){TOKEN_ASSIGN, make_none()};
        case '\0':
        case '\n': return (Token){TOKEN_EOF, make_none()};
        default:
            printf("Unexpected char: %c\n", c);
            exit(1);
    }

    return (Token){TOKEN_EOF, 0};
}
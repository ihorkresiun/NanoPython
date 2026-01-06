#include "lexer.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"


Token lexer_next(Lexer * lexer)
{
    // Skip whitespace
    while (isspace(lexer->input[lexer->pos])) lexer->pos++;

    char c = lexer->input[lexer->pos];

    if (c == '\0') return (Token){TOKEN_EOF, 0};

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
        tok.value = atof(buffer);
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
        case '+': return (Token){TOKEN_PLUS, 0};
        case '-': return (Token){TOKEN_MINUS, 0};
        case '*': return (Token){TOKEN_STAR, 0};
        case '/': return (Token){TOKEN_SLASH, 0};
        case '(': return (Token){TOKEN_LPAREN, 0};
        case ')': return (Token){TOKEN_RPAREN, 0};
        case '^': return (Token){TOKEN_CARET, 0};
        case '=': return (Token){TOKEN_ASSIGN, 0};
        case '\0':
        case '\n': return (Token){TOKEN_EOF, 0};

        default:
            printf("Unexpected char: %c\n", c);
            exit(1);
    }

    return (Token){TOKEN_EOF, 0};
}
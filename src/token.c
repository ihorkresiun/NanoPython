#include "token.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"


static void lexer_advance(Lexer * lexer)
{
    lexer->pos++;
    lexer->current = lexer->input[lexer->pos];
}

Token lexer_next(Lexer * lexer)
{
    // Skip whitespace
    while(lexer->current != '\0' && lexer->current == ' ') {
        lexer_advance(lexer);
    }

    if(isdigit(lexer->current) || lexer->current == '.') {
        // Parse number
        char buffer[64];
        size_t i = 0;
        while(isdigit(lexer->current) || lexer->current == '.') {
            buffer[i++] = lexer->current;
            lexer_advance(lexer);
        }
        buffer[i] = '\0';
        Token tok;
        tok.type = TOKEN_NUMBER;
        tok.value = atof(buffer);
        return tok;
    }

    char c = lexer->current;
    lexer_advance(lexer);

    switch(c) {
        case '+': return (Token){TOKEN_PLUS, 0};
        case '-': return (Token){TOKEN_MINUS, 0};
        case '*': return (Token){TOKEN_STAR, 0};
        case '/': return (Token){TOKEN_SLASH, 0};
        case '(': return (Token){TOKEN_LPAREN, 0};
        case ')': return (Token){TOKEN_RPAREN, 0};
        case '\0':
        case '\n': return (Token){TOKEN_EOF, 0};

        default:
            printf("Unexpected char: %c\n", c);
            exit(1);
    }

    return (Token){TOKEN_EOF, 0};
}
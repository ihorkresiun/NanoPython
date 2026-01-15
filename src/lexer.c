#include "lexer.h"
#include "vars.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

typedef struct Keyword{
    const char* name;
    TokenType type;
} Keyword;

static Keyword keywords[] = {
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"def", TOKEN_DEF},
    {"while", TOKEN_WHILE},
    {"print", TOKEN_PRINT},
    {"and", TOKEN_AND},
    {"or", TOKEN_OR},
    {"not", TOKEN_NOT},
    {"return", TOKEN_RETURN},

    {NULL, 0}
};

static TokenType check_keyword(const char* ident) {
    for (int i = 0; keywords[i].name != NULL; i++) {
        if (strcmp(ident, keywords[i].name) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENT;
}

void lexer_init(Lexer * lexer, const char * input) {
    lexer->input = input;
    lexer->pos = 0;
    lexer->line_start = 1;
    lexer->indent_top = 0;
    lexer->indent_stack[lexer->indent_top] = 0;
}

static int count_leading_spaces(const char * str) {
   int count = 0;
    while (str[count] == ' ') count++;
    return count;
}

Token lexer_next(Lexer* l) {
    Token tok = {0};

    if (l->pending_indents > 0) {
        l->pending_indents--;
        tok.type = TOKEN_INDENT;
        return tok;
    }

    if (l->pending_dedents > 0) {
        l->pending_dedents--;
        tok.type = TOKEN_DEDENT;
        return tok;
    }

    char s = l->input[l->pos];

    // EOF
    if (s == '\0') {
        tok.type = TOKEN_EOF;
        return tok;
    }

    // New line and indentation handling
    if (s == '\n') {
        l->pos++;

        // Check for indentation in the next line
        const char* next_line = l->input + l->pos;
        int spaces = count_leading_spaces(next_line);
        int current_indent = l->indent_stack[l->indent_top];

        if (spaces > current_indent) {
            l->indent_stack[++l->indent_top] = spaces;
            l->pending_indents++;
        } else if (spaces < l->indent_stack[l->indent_top]) {
            l->indent_top--;
            l->pending_dedents++;
        }

        tok.type = TOKEN_NEWLINE;
        return tok;
    }

    if (s == '\r') {
        l->pos++;
        return lexer_next(l);
    }

    if (s == '#') {
        // Skip comment till end of line
        while (s != '\n' && s != '\0') {
            l->pos++;
            s = l->input[l->pos];
        }
        return lexer_next(l);
    }

    if (s == '"') {
        // Parse string literal
        l->pos++; // skip opening quote
        char buffer[256];
        size_t i = 0;
        s = l->input[l->pos];
        while (s != '"' && s != '\0' && i < sizeof(buffer) - 1) {
            buffer[i++] = s;
            l->pos++;
            s = l->input[l->pos];
        }
        buffer[i] = '\0';
        if (s == '"') {
            l->pos++; // skip closing quote
        } else {
            printf("Unterminated string literal\n");
            exit(1);
        }
        tok.type = TOKEN_STRING; // Using STRING type for strings
        tok.value = make_string(buffer);
        return tok;
    }

    // Skip spaces inside a line
    if (s == ' ' || s == '\t') { l->pos++; return lexer_next(l); }

    // Numbers
    if(isdigit(s) || s == '.') {
        // Parse number
        char buffer[64];
        size_t i = 0;
        while(isdigit(s) || s == '.') {
            buffer[i++] = l->input[l->pos++];
            s = l->input[l->pos];
        }
        buffer[i] = '\0';
        tok.type = TOKEN_NUMBER;
        tok.value = make_number(atof(buffer));
        return tok;
    }

    // Identifier or keyword
    if (isalpha(s)) {
        char buffer[64];
        size_t i = 0;
        while (isalpha(s) || isdigit(s) || s == '_') {
            buffer[i++] = l->input[l->pos++];
            s = l->input[l->pos];
        }
        buffer[i] = 0;
        tok.type = check_keyword(buffer);
        tok.ident = strdup(buffer);
        return tok;
    }

    if (s == '<') {
        l->pos++;
        if (l->input[l->pos] == '=') {
            l->pos++;
            tok.type = TOKEN_LE;
        } else {
            tok.type = TOKEN_LT;
        }
        return tok;
    }
    if (s == '>') {
        l->pos++;
        if (l->input[l->pos] == '=') {
            l->pos++;
            tok.type = TOKEN_GE;
        } else {
            tok.type = TOKEN_GT;
        }
        return tok;
    }
    if (s == '=') {
        l->pos++;
        if (l->input[l->pos] == '=') {
            l->pos++;
            tok.type = TOKEN_EQ;
        } else {
            tok.type = TOKEN_ASSIGN;
        }
        return tok;
    }
    if (s == '!') {
        l->pos++;
        if (l->input[l->pos] == '=') {
            l->pos++;
            tok.type = TOKEN_NE;;
            return tok;
        }
    }

    // Operators
    switch (s) {
        case '+': tok.type=TOKEN_PLUS;      l->pos++; return tok;
        case '-': tok.type=TOKEN_MINUS;     l->pos++; return tok;
        case '*': tok.type=TOKEN_STAR;      l->pos++; return tok;
        case '/': tok.type=TOKEN_SLASH;     l->pos++; return tok;
        case '^': tok.type=TOKEN_CARET;     l->pos++; return tok;
        case '(': tok.type=TOKEN_LPAREN;    l->pos++; return tok;
        case ')': tok.type=TOKEN_RPAREN;    l->pos++; return tok;
        case ',': tok.type=TOKEN_COMMA;     l->pos++; return tok;
        case ':': tok.type=TOKEN_COLON;     l->pos++; return tok;
    }

    printf("Unknown char: %c\n", s);
    exit(1);
}

Token lexer_peek_next(Lexer* l) {
    int saved_pos = l->pos;
    Token tok = lexer_next(l);
    l->pos = saved_pos;
    return tok;
}
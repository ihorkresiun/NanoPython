#ifndef __INC_TOKEN_H__
#define __INC_TOKEN_H__

#include "stddef.h"
#include "token.h"

typedef struct {
    const char * input;
    size_t pos;
    char current;
} Lexer;

Token lexer_next(Lexer * lexer);

#endif // __INC_TOKEN_H__

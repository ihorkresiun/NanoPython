#ifndef __INC_EVAL_H__
#define __INC_EVAL_H__

#include "ast.h"

#include "vars.h"

typedef enum {
    NORMAL,
    RETURNING,
    BREAKING,
    CONTINUING
} EvalStatus;

typedef struct EvalResult {
    Value value;
    EvalStatus status;
} EvalResult;

EvalResult eval(Ast* e, Scope* scope);

#endif // __INC_EVAL_H__
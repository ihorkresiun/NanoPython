#ifndef __INC_EVAL_H__
#define __INC_EVAL_H__

#include "ast.h"

#include "vars.h"

Value eval(Ast* e, Scope* scope);

#endif // __INC_EVAL_H__
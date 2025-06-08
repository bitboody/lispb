#ifndef EVALUATION_H
#define EVALUATION_H

#include "lib/mpc.h"
#include "error_handling.h"

lval eval_op(lval x, char *op, lval y);
lval eval(mpc_ast_t *t);

#endif
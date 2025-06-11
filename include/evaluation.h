#ifndef EVALUATION_H
#define EVALUATION_H

#include "../lib/mpc.h"
#include "lval.h"

lval *lval_eval(lval *v);
lval *lval_eval_sexpr(lval *v);
lval *builtin_op(lval *a, char *op);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);

#endif
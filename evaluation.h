#ifndef EVALUATION_H
#define EVALUATION_H

#include "lib/mpc.h"
#include "error_handling.h"

lval *lval_add(lval *v, lval *x);
lval *lval_read(mpc_ast_t *t);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_eval_sexpr(lval *v);
lval *lval_eval(lval *v);
lval *lval_pop(lval *v, int i);
lval *lval_take(lval *v, int i);
lval *builtin_op(lval *a, char *op);

#endif
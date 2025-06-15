#ifndef EVALUATION_H
#define EVALUATION_H

#include "../lib/mpc.h"
#include "lval.h"

lval *lval_eval(lval *v);
lval *lval_eval_sexpr(lval *v);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *builtin_op(lval *a, char *op);
lval *builtin_head(lval *a);
lval *builtin_tail(lval *a);
lval *builtin_list(lval *a);
lval *builtin_eval(lval *a);
lval *builtin_join(lval *a);
lval *lval_join(lval *x, lval *y);
lval *builtin_cons(lval *a);
lval *builtin_len(lval *a);
lval *builtin(lval *a, char *func);

#endif
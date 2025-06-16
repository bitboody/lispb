#ifndef EVALUATION_H
#define EVALUATION_H

#include "../lib/mpc.h"
#include "lval.h"

lval *lval_eval(lenv *e, lval *v);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *builtin_op_internal(lenv *e, lval *a, const char *op);
lval *builtin_op(lenv *e, lval *a, char *op);
lval *builtin_init(lenv *e, lval *a);
lval *builtin(lenv *e, lval *a, char *func);
lval *builtin_head(lenv *e, lval *a);
lval *builtin_tail(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_join(lenv *e, lval *a);
lval *builtin_cons(lenv *e, lval *a);
lval *builtin_len(lenv *e, lval *a);
lval *lval_join(lenv *e, lval *x, lval *y);
lval *builtin_def(lenv *e, lval *a);

#endif
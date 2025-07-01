#ifndef EVALUATION_H
#define EVALUATION_H

#include "../lib/mpc.h"
#include "lval.h"

#define LASSERT(args, cond, fmt, ...)             \
    if (!(cond))                                  \
    {                                             \
        lval *err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args);                           \
        return err;                               \
    }

#define LASSERT_NUM(func, args, expected)                                           \
    if ((args)->count != (expected))                                                \
    {                                                                               \
        lval *err = lval_err(                                                       \
            "Function '%s' passed wrong number of arguments. Got %i, Expected %i.", \
            (func), (args)->count, (expected));                                     \
        lval_del(args);                                                             \
        return err;                                                                 \
    }

#define LASSERT_TYPE(func, args, index, expected)                                        \
    if ((args)->cell[(index)]->type != (expected))                                       \
    {                                                                                    \
        lval *err = lval_err(                                                            \
            "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
            (func), (index),                                                             \
            ltype_name((args)->cell[(index)]->type), ltype_name((expected)));            \
        lval_del(args);                                                                  \
        return err;                                                                      \
    }

lval *lval_eval(lenv *e, lval *v);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read_str(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *builtin_op_internal(lenv *e, lval *a, const char *op);
lval *builtin_op(lenv *e, lval *a, char *op);
lval *builtin_lambda(lenv *e, lval *a);
lval *builtin_init(lenv *e, lval *a);
lval *builtin(lenv *e, lval *a, char *func);
lval *builtin_head(lenv *e, lval *a);
lval *builtin_tail(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_join(lenv *e, lval *a);
lval *builtin_cons(lenv *e, lval *a);
lval *builtin_len(lenv *e, lval *a);
lval *builtin_load(lenv *e, lval *a);
lval *builtin_print(lenv *e, lval *a);
lval *builtin_error(lenv *e, lval *a);
lval *lval_join(lenv *e, lval *x, lval *y);
lval *builtin_def(lenv *e, lval *a);
lval *builtin_var(lenv *e, lval *a, char *func);
lval *builtin_ord(lenv *e, lval *a, char *op);
lval *builtin_cmp(lenv *e, lval *a, char *op);
lval *builtin_if(lenv *e, lval *a);

#endif
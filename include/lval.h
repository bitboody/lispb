#ifndef LVAL_H
#define LVAL_H

#include "../lib/mpc.h"

typedef struct lval {
    int type;
    char *err;

    union {
        long num;
        double dnum;
        char *sym;
    } data;

    int count;
    struct lval **cell;
} lval;

/* Possible lval types */
enum {
    LVAL_LONG,
    LVAL_DOUBLE,
    LVAL_SYM,
    LVAL_SEXPR,
    LVAL_ERR
};

/* Possible error types*/
enum
{
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

/* Constructors */
lval *lval_long(long x);
lval *lval_double(double x);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_err(char *m);

/* Manipulation */
void lval_del(lval *v);
lval *lval_pop(lval *v, int i);
lval *lval_take(lval *v, int i);
lval *lval_add(lval *v, lval *x);

/* IO / Parsing */
void lval_print(lval *v);
void lval_println(lval *v);
void lval_expr_print(lval *v, char open, char close);

#endif
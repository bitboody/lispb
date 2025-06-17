#ifndef LVAL_H
#define LVAL_H

#include "../lib/mpc.h"

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// Lisp Value
typedef lval *(*lbuiltin)(lenv *, lval *);

typedef struct lval
{
    int type;
    union
    {
        long num;
        double dnum;
        char *err;
        char *sym;
        lbuiltin fun;
    } data;

    char *func_name;
    int count;
    struct lval **cell;
} lval;

struct lenv
{
    int count;
    char **syms;
    lval **vals;
};

/* Possible lval types */
enum
{
    LVAL_LONG,
    LVAL_DOUBLE,
    LVAL_SYM,
    LVAL_FUN,
    LVAL_SEXPR,
    LVAL_QEXPR,
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
lval *lval_fun(lbuiltin func, const char *name);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_err(char *fmt, ...);
lenv *lenv_new(void);
lval *builtin_add(lenv *e, lval *a);
lval *builtin_sub(lenv *e, lval *a);
lval *builtin_mul(lenv *e, lval *a);
lval *builtin_div(lenv *e, lval *a);
lval *builtin_mod(lenv *e, lval *a);
lval *builtin_min(lenv *e, lval *a);
lval *builtin_max(lenv *e, lval *a);

/* Manipulation */
void lval_del(lval *v);
lval *lval_pop(lval *v, int i);
lval *lval_take(lval *v, int i);
lval *lval_add(lval *v, lval *x);
lval *lval_copy(lval *v);
void lenv_del(lenv *e);
lval *lenv_get(lenv *e, lval *k);
void lenv_put(lenv *e, lval *k, lval *v);
void lenv_add_builtins(lenv *e);

/* IO / Parsing */
void lval_print(lval *v);
void lval_println(lval *v);
void lval_expr_print(lval *v, char open, char close);
char *ltype_name(int t);

#endif
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
    lenv *env;
    union
    {
        // Basic
        long num;
        double dnum;
        char *err;
        char *sym;
        char *str;

        // Function
        struct
        {
            lbuiltin builtin;
            lval *formals;
            lval *body;
        };
    } data;

    char *func_name;
    int count;
    struct lval **cell;
} lval;

struct lenv
{
    lenv *par;
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
    LVAL_STR,
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
lval *lval_str(char *s);
lval *lval_fun(lbuiltin func, const char *name);
lval *lval_lambda(lval *formals, lval *body);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_err(char *fmt, ...);
lenv *lenv_new(void);

/* Builtins */
lval *builtin_add(lenv *e, lval *a);
lval *builtin_sub(lenv *e, lval *a);
lval *builtin_mul(lenv *e, lval *a);
lval *builtin_div(lenv *e, lval *a);
lval *builtin_mod(lenv *e, lval *a);
lval *builtin_min(lenv *e, lval *a);
lval *builtin_max(lenv *e, lval *a);
lval *builtin_gt(lenv *e, lval *a);
lval *builtin_lt(lenv *e, lval *a);
lval *builtin_ge(lenv *e, lval *a);
lval *builtin_le(lenv *e, lval *a);
lval *builtin_eq(lenv *e, lval *a);
lval *builtin_ne(lenv *e, lval *a);
lval *builtin_or(lenv *e, lval *a);
lval *builtin_and(lenv *e, lval *a);
lval *builtin_not(lenv *e, lval *a);

int lval_eq(lval *x, lval *y);
int lval_or(lval *x, lval *y);
int lval_and(lval *x, lval *y);

/* Manipulation */
void lval_del(lval *v);
lval *lval_pop(lval *v, int i);
lval *lval_take(lval *v, int i);
lval *lval_add(lval *v, lval *x);
lval *lval_copy(lval *v);
lenv *lenv_copy(lenv *e);
lval *lval_call(lenv *e, lval *f, lval *a);
void lenv_del(lenv *e);
lval *lenv_get(lenv *e, lval *k);
void lenv_put(lenv *e, lval *k, lval *v);
void lenv_def(lenv *e, lval *k, lval *v);
lval *lval_join(lenv *e, lval *x, lval *y);
void lenv_add_builtins(lenv *e);

/* IO / Parsing */
void lval_print(lval *v);
void lval_println(lval *v);
void lval_expr_print(lval *v, char open, char close);
void lval_print_str(lval *v);
char *ltype_name(int t);
int lval_is_true(lval *v);

#endif
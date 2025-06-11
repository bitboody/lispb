#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

typedef struct lval
{
    int type;
    char *err;
    union
    {
        long num;
        double dnum;
        char *sym;
    } data;
    int count;
    struct lval **cell;
} lval;

/* Possible lval types*/
enum
{
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

lval *lval_long(long x);
lval *lval_double(double x);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_err(char *m);
void lval_del(lval *v);
void lval_expr_print(lval *v, char open, char close);
void lval_print(lval *v);
void lval_println(lval *v);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handling.h"

lval *lval_sym(char *s)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->data.sym = malloc(strlen(s) + 1);
    strcpy(v->data.sym, s);
    return v;
}

lval *lval_sexpr(void)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval *lval_long(long x)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_LONG;
    v->data.num = x;
    return v;
}

lval *lval_double(double x)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_DOUBLE;
    v->data.dnum = x;
    return v;
}

lval *lval_err(char *m)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

void lval_del(lval *v)
{
    switch (v->type)
    {
    case LVAL_LONG:
        break;
    case LVAL_DOUBLE:
        break;
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->data.sym);
        break;
    case LVAL_SEXPR:
        for (int i = 0; i < v->count; i++)
        {
            lval_del(v->cell[i]);
        }
        free(v->cell);
        break;
    }
    free(v);
}

void lval_expr_print(lval *v, char open, char close)
{
    putchar(open);
    for (int i = 0; i < v->count; i++)
    {
        lval_print(v->cell[i]);

        if (i != (v->count - 1))
        {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval *v)
{
    switch (v->type)
    {
    case LVAL_LONG:
        printf("%li", v->data.num);
        break;
    case LVAL_DOUBLE:
        printf("%lf", v->data.dnum);
        break;
    case LVAL_SYM:
        printf("%s", v->data.sym);
        break;
    case LVAL_SEXPR:
        lval_expr_print(v, '(', ')');
        break;
    case LVAL_ERR:
        printf("Error: %s", v->err);
        break;
    }
}

void lval_println(lval *v)
{
    lval_print(v);
    putchar('\n');
}
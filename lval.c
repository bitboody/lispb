#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/lval.h"
#include "include/evaluation.h"

lval *lval_pop(lval *v, int i)
{
    lval *x = v->cell[i];
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    return x;
}

lval *lval_take(lval *v, int i)
{
    lval *x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval *lval_add(lval *v, lval *x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval *lval_copy(lval *v)
{
    lval *x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type)
    {
    // Copy lists by copying each sub-expression
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        x->count = v->count;
        x->cell = malloc(sizeof(lval *) * x->count);
        for (int i = 0; i < x->count; i++)
        {
            x->cell[i] = lval_copy(v->cell[i]);
        }
        break;
    case LVAL_FUN:
        x->data.fun = v->data.fun;
        break;
    case LVAL_LONG:
        x->data.num = v->data.num;
        break;
    case LVAL_DOUBLE:
        x->data.dnum = v->data.dnum;
        break;
    case LVAL_SYM:
        x->data.sym = malloc(strlen(v->data.sym) + 1);
        strcpy(x->data.sym, v->data.sym);
        break;
    case LVAL_ERR:
        x->data.err = malloc(strlen(v->data.err) + 1);
        strcpy(x->data.err, v->data.err);
        break;
    }
    return x;
}

void lval_del(lval *v)
{
    switch (v->type)
    {
    case LVAL_LONG:
    case LVAL_DOUBLE:
    case LVAL_FUN:
        break;
    case LVAL_ERR:
        free(v->data.err);
        break;
    case LVAL_SYM:
        free(v->data.sym);
        break;
    case LVAL_QEXPR:
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

void lenv_del(lenv *e)
{
    for (int i = 0; i < e->count; i++)
    {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval *lenv_get(lenv *e, lval *k)
{
    for (int i = 0; i < e->count; i++)
    {
        if (strcmp(e->syms[i], k->data.sym) == 0)
        {
            return lval_copy(e->vals[i]);
        }
    }
    return lval_err("Unbound Symbol '%s'", k->data.sym);
}

void lenv_put(lenv *e, lval *k, lval *v)
{
    for (int i = 0; i < e->count; i++)
    {
        if (strcmp(e->syms[i], k->data.sym) == 0)
        {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval *) * e->count);
    e->syms = realloc(e->syms, sizeof(lval *) * e->count);

    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->data.sym) + 1);
    strcpy(e->syms[e->count - 1], k->data.sym);
}

lval *builtin_add(lenv *e, lval *a) { return builtin_op_internal(e, a, "+"); }
lval *builtin_sub(lenv *e, lval *a) { return builtin_op_internal(e, a, "-"); }
lval *builtin_mul(lenv *e, lval *a) { return builtin_op_internal(e, a, "*"); }
lval *builtin_div(lenv *e, lval *a) { return builtin_op_internal(e, a, "/"); }
lval *builtin_mod(lenv *e, lval *a) { return builtin_op_internal(e, a, "%"); }
lval *builtin_min(lenv *e, lval *a) { return builtin_op_internal(e, a, "min"); }
lval *builtin_max(lenv *e, lval *a) { return builtin_op_internal(e, a, "max"); }

void lenv_add_builtin(lenv *e, char *name, lbuiltin func)
{
    lval *k = lval_sym(name);
    lval *v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

void lenv_add_builtins(lenv *e)
{
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "init", builtin_init);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "def",  builtin_def);

    // Arithmetic operators
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "%", builtin_mod);
    lenv_add_builtin(e, "min", builtin_min);
    lenv_add_builtin(e, "max", builtin_max);
}

// Constructors
lval *lval_sym(char *s)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->data.sym = malloc(strlen(s) + 1);
    strcpy(v->data.sym, s);
    return v;
}

lval *lval_fun(lbuiltin func)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->data.fun = func;
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

lval *lval_qexpr(void)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
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

lenv *lenv_new(void)
{
    lenv *e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
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
    case LVAL_FUN:
        printf("<function>");
        break;
    case LVAL_SEXPR:
        lval_expr_print(v, '(', ')');
        break;
    case LVAL_QEXPR:
        lval_expr_print(v, '{', '}');
        break;
    case LVAL_ERR:
        printf("Error: %s", v->data.err);
        break;
    }
}

void lval_println(lval *v)
{
    lval_print(v);
    putchar('\n');
}
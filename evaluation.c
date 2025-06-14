#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "lib/mpc.h"
#include "include/lval.h"
#include "include/evaluation.h"

#define LASSERT(args, cond, err) \
    if (!(cond))                 \
    {                            \
        lval_del(args);          \
        return lval_err(err);    \
    }

lval *lval_read_num(mpc_ast_t *t)
{
    errno = 0;
    if (strchr(t->contents, '.'))
    {
        double d = strtod(t->contents, NULL);
        return errno != ERANGE ? lval_double(d) : lval_err("invalid number");
    }
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_long(x) : lval_err("invalid number");
}

lval *lval_read(mpc_ast_t *t)
{
    if (strstr(t->tag, "number"))
        return lval_read_num(t);
    if (strstr(t->tag, "symbol"))
        return lval_sym(t->contents);

    lval *x = NULL;
    if (strcmp(t->tag, ">") == 0)
        x = lval_sexpr();
    if (strstr(t->tag, "sexpr"))
        x = lval_sexpr();
    if (strstr(t->tag, "qexpr"))
        x = lval_qexpr();

    for (int i = 0; i < t->children_num; i++)
    {
        if (strstr(t->children[i]->tag, "comment"))
            continue;
        if (strcmp(t->children[i]->contents, "(") == 0)
            continue;
        if (strcmp(t->children[i]->contents, ")") == 0)
            continue;
        if (strcmp(t->children[i]->contents, "{") == 0)
            continue;
        if (strcmp(t->children[i]->contents, "}") == 0)
            continue;
        if (strcmp(t->children[i]->tag, "regex") == 0)
            continue;

        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

lval *lval_add(lval *v, lval *x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval *lval_eval_sexpr(lval *v)
{
    for (int i = 0; i < v->count; i++)
        v->cell[i] = lval_eval(v->cell[i]);

    for (int i = 0; i < v->count; i++)
    {
        if (v->cell[i]->type == LVAL_ERR)
            return lval_take(v, i);
    }

    if (v->count == 0)
        return v;

    if (v->count == 1)
        return lval_take(v, 0);

    lval *f = lval_pop(v, 0);
    if (f->type != LVAL_SYM)
    {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with symbol");
    }

    lval *result = builtin(v, f->data.sym);
    lval_del(f);
    return result;
}

lval *lval_eval(lval *v)
{
    if (v->type == LVAL_SEXPR)
        return lval_eval_sexpr(v);
    return v;
}

lval *builtin_op(lval *a, char *op)
{
    for (int i = 0; i < a->count; i++)
    {
        if (a->cell[i]->type != LVAL_LONG && a->cell[i]->type != LVAL_DOUBLE)
        {
            lval_del(a);
            return lval_err("Cannot operate on non-number");
        }
    }

    lval *x = lval_pop(a, 0);

    if ((strcmp(op, "-") == 0) && a->count == 0)
    {
        if (x->type == LVAL_LONG)
            x->data.num = -x->data.num;
        else
            x->data.dnum = -x->data.dnum;
    }

    while (a->count > 0)
    {
        lval *y = lval_pop(a, 0);

        int is_double = (x->type == LVAL_DOUBLE || y->type == LVAL_DOUBLE);

        double xnum = (x->type == LVAL_DOUBLE) ? x->data.dnum : (double)x->data.num;
        double ynum = (y->type == LVAL_DOUBLE) ? y->data.dnum : (double)y->data.num;

        double res = 0;

        if (strcmp(op, "+") == 0)
            res = xnum + ynum;
        else if (strcmp(op, "-") == 0)
            res = xnum - ynum;
        else if (strcmp(op, "*") == 0)
            res = xnum * ynum;
        else if (strcmp(op, "/") == 0)
        {
            if (ynum == 0)
            {
                lval_del(x);
                lval_del(y);
                return lval_err("Division by Zero");
            }
            res = xnum / ynum;
        }
        else if (strcmp(op, "%") == 0)
        {
            if (is_double)
            {
                lval_del(x);
                lval_del(y);
                lval_del(a);
                return lval_err("Modulo operator not supported for double type");
            }
            if (y->data.num == 0)
            {
                lval_del(x);
                lval_del(y);
                lval_del(a);
                return lval_err("Division by Zero");
            }
            res = x->data.num % y->data.num;
        }
        else if (strcmp(op, "min") == 0)
        {
            res = xnum;
            if (xnum > ynum)
                res = ynum;
        }
        else if (strcmp(op, "max") == 0)
        {
            res = xnum;
            if (xnum < ynum)
                res = ynum;
        }
        if (is_double)
        {
            x->type = LVAL_DOUBLE;
            x->data.dnum = res;
        }
        else
        {
            x->type = LVAL_LONG;
            x->data.num = (long)res;
        }
        lval_del(y);
    }
    lval_del(a);
    return x;
}

lval *builtin(lval *a, char *func)
{
    if (strcmp("list", func) == 0)
        return builtin_list(a);
    if (strcmp("head", func) == 0)
        return builtin_head(a);
    if (strcmp("tail", func) == 0)
        return builtin_tail(a);
    if (strcmp("join", func) == 0)
        return builtin_join(a);
    if (strcmp("eval", func) == 0)
        return builtin_eval(a);
    if (strstr("+-/*minmax", func))
        return builtin_op(a, func);
    lval_del(a);
    return lval_err("Unknown function");
}

lval *builtin_head(lval *a)
{
    LASSERT(a, a->count == 1, "Function 'head' passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type");
    LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}");

    lval *v = lval_take(a, 0);

    while (v->count > 1)
        lval_del(lval_pop(v, 1));

    return v;
}

lval *builtin_tail(lval *a)
{
    LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type");
    LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

    lval *v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));

    return v;
}

lval *builtin_list(lval *a)
{
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_eval(lval *a)
{
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type");

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval *builtin_join(lval *a)
{
    for (int i = 0; i < a->count; i++)
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type");

    lval *x = lval_pop(a, 0);

    while (a->count)
    {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

lval *lval_join(lval *x, lval *y)
{
    while (y->count)
        x = lval_add(x, lval_pop(y, 0));

    lval_del(y);
    return x;
}
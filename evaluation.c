#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "lib/mpc.h"
#include "include/lval.h"
#include "include/evaluation.h"

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

lval *lval_read_num(mpc_ast_t *t)
{
    errno = 0;
    if (strchr(t->contents, '.'))
    {
        double d = strtod(t->contents, NULL);
        return errno != ERANGE ? lval_double(d) : lval_err("Invalid number.");
    }
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_long(x) : lval_err("Invalid number.");
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

lval *lval_eval_sexpr(lenv *e, lval *v)
{
    for (int i = 0; i < v->count; i++)
        v->cell[i] = lval_eval(e, v->cell[i]);

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
    if (f->type != LVAL_FUN)
    {
        lval_del(f);
        lval_del(v);
        return lval_err("S-Expression starts with incorrect type. Got %s, Expected %s.",
                        ltype_name(f->type), ltype_name(LVAL_FUN));
    }

    // lval *result = f->data.builtin(e, v);
    lval *result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval *lval_eval(lenv *e, lval *v)
{
    if (v->type == LVAL_SYM)
    {
        lval *x = lenv_get(e, v);
        lval_del(v);
        return x;
    }

    if (v->type == LVAL_SEXPR)
    {
        return lval_eval_sexpr(e, v);
    }

    return v;
}

lval *builtin_op_internal(lenv *e, lval *a, const char *op)
{
    for (int i = 0; i < a->count; i++)
    {
        if (a->cell[i]->type != LVAL_LONG && a->cell[i]->type != LVAL_DOUBLE)
        {
            lval *err = lval_err("Function '%s' passed incorrect type at argument %i. Got %s, Expected %s or %s.", op, i, ltype_name(a->cell[i]->type), ltype_name(LVAL_LONG), ltype_name(LVAL_DOUBLE));
            lval_del(a);
            return err;
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
                return lval_err("Division by zero.");
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
                return lval_err("Modulo not supported for double.");
            }
            if (y->data.num == 0)
            {
                lval_del(x);
                lval_del(y);
                lval_del(a);
                return lval_err("Modulo by zero.");
            }
            res = x->data.num % y->data.num;
        }
        else if (strcmp(op, "min") == 0)
        {
            res = (xnum < ynum) ? xnum : ynum;
        }
        else if (strcmp(op, "max") == 0)
        {
            res = (xnum > ynum) ? xnum : ynum;
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

lval *builtin_op(lenv *e, lval *a, char *op)
{
    return builtin_op_internal(e, a, op);
}

lval *builtin(lenv *e, lval *a, char *func)
{
    if (strcmp("eval", func) == 0)
        return builtin_eval(e, a);
    if (strcmp("list", func) == 0)
        return builtin_list(e, a);
    if (strcmp("head", func) == 0)
        return builtin_head(e, a);
    if (strcmp("tail", func) == 0)
        return builtin_tail(e, a);
    if (strcmp("join", func) == 0)
        return builtin_join(e, a);
    if (strcmp("cons", func) == 0)
        return builtin_cons(e, a);
    if (strcmp("len", func) == 0)
        return builtin_len(e, a);
    if (strcmp("init", func) == 0)
        return builtin_init(e, a);
    if (strstr("+-/*minmax", func))
        return builtin_op_internal(e, a, func);
    lval_del(a);
    return lval_err("Unknown function.");
}

lval *builtin_lambda(lenv *e, lval *a)
{
    LASSERT_NUM("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; i++)
    {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
                "Cannot define non-symbol. Got %s, Expected %s.",
                ltype_name(a->cell[0]->cell[i]->type),
                ltype_name(LVAL_SYM));
    }

    lval *formals = lval_pop(a, 0);
    lval *body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval *builtin_var(lenv *e, lval *a, char *func)
{
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval *syms = a->cell[0];
    for (int i = 0; i < syms->count; i++)
    {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
                "Function '%s' cannot define non-symbol. Got %s, Expected %s.",
                func,
                ltype_name(syms->cell[i]->type),
                ltype_name(LVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count - 1),
            "Function '%s' passed too many arguments for symbols. Got %i, Expected %i.",
            func,
            syms->count,
            a->count - 1);

    for (int i = 0; i < syms->count; i++)
    {
        if (strcmp(func, "def") == 0)
            lenv_def(e, syms->cell[i], a->cell[i + 1]);

        if (strcmp(func, "=") == 0)
            lenv_def(e, syms->cell[i], a->cell[i + 1]);
    }

    lval_del(a);
    return lval_sexpr();
}

lval *builtin_head(lenv *e, lval *a)
{
    LASSERT(a, a->count == 1, "Function 'head' passed too many arguments. Got %i, Expected %i.", a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}");

    lval *v = lval_take(a, 0);

    while (v->count > 1)
        lval_del(lval_pop(v, 1));

    return v;
}

lval *builtin_tail(lenv *e, lval *a)
{
    LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments. Got %i, Expected %i.", a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}.");

    lval *v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));

    return v;
}

lval *builtin_list(lenv *e, lval *a)
{
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_eval(lenv *e, lval *a)
{
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments.");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval *builtin_cons(lenv *e, lval *a)
{
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'cons' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval *list = lval_qexpr();
    list = lval_add(list, a->cell[0]);
    list = lval_join(e, list, a->cell[1]);

    lval_del(a);
    return list;
}

lval *builtin_len(lenv *e, lval *a)
{
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'len' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    lval *x = lval_qexpr();

    long count = 0;
    for (int i = 0; i < a->count; i++)
    {
        count += a->cell[i]->count;
    }

    x->type = LVAL_LONG;
    x->data.num = count;

    lval_del(a);

    return x;
}

lval *builtin_init(lenv *e, lval *a)
{
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval *x = a->cell[0];
    LASSERT(x, x->count != 0, "Function 'init' passed too little arguments.");
    lval_take(x, x->count - 1);
    x->type = LVAL_QEXPR;
    lval_del(a);
    return x;
}

lval *builtin_join(lenv *e, lval *a)
{
    for (int i = 0; i < a->count; i++)
        LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'join' passed incorrect type. Got %s, Expected %s.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval *x = lval_pop(a, 0);

    while (a->count)
    {
        x = lval_join(e, x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

lval *lval_join(lenv *e, lval *x, lval *y)
{
    while (y->count)
        x = lval_add(x, lval_pop(y, 0));

    lval_del(y);
    return x;
}
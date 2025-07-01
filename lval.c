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
        if (v->data.builtin)
        {
            x->data.builtin = v->data.builtin;
            x->data.builtin = v->data.builtin;
            x->func_name = malloc(strlen(v->func_name) + 1);
            strcpy(x->func_name, v->func_name);
        }
        else
        {
            x->data.builtin = NULL;
            x->env = lenv_copy(v->env);
            x->data.formals = lval_copy(v->data.formals);
            x->data.body = lval_copy(v->data.body);
        }
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
    case LVAL_STR:
        x->data.str = malloc(strlen(v->data.str) + 1);
        strcpy(x->data.str, v->data.str);
        break;
    case LVAL_ERR:
        x->data.err = malloc(strlen(v->data.err) + 1);
        strcpy(x->data.err, v->data.err);
        break;
    }
    return x;
}

lenv *lenv_copy(lenv *e)
{
    lenv *n = malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char *) * n->count);
    n->vals = malloc(sizeof(lval *) * n->count);
    for (int i = 0; i < e->count; i++)
    {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}

lval *lval_call(lenv *e, lval *f, lval *a)
{

    /* If Builtin then simply apply that */
    if (f->data.builtin)
    {
        return f->data.builtin(e, a);
    }

    /* Record Argument Counts */
    int given = a->count;
    int total = f->data.formals->count;

    /* While arguments still remain to be processed */
    while (a->count)
    {

        /* If we've ran out of formal arguments to bind */
        if (f->data.formals->count == 0)
        {
            lval_del(a);
            return lval_err("Function passed too many arguments. "
                            "Got %i, Expected %i.",
                            given, total);
        }

        /* Pop the first symbol from the formals */
        lval *sym = lval_pop(f->data.formals, 0);

        /* Special Case to deal with '&' */
        if (strcmp(sym->data.sym, "&") == 0)
        {

            /* Ensure '&' is followed by another symbol */
            if (f->data.formals->count != 1)
            {
                lval_del(a);
                return lval_err("Function format invalid. "
                                "Symbol '&' not followed by single symbol.");
            }

            /* Next formal should be bound to remaining arguments */
            lval *nsym = lval_pop(f->data.formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        /* Pop the next argument from the list */
        lval *val = lval_pop(a, 0);

        /* Bind a copy into the function's environment */
        lenv_put(f->env, sym, val);

        /* Delete symbol and value */
        lval_del(sym);
        lval_del(val);
    }

    /* Argument list is now bound so can be cleaned up */
    lval_del(a);

    /* If '&' remains in formal list bind to empty list */
    if (f->data.formals->count > 0 &&
        strcmp(f->data.formals->cell[0]->data.sym, "&") == 0)
    {

        /* Check to ensure that & is not passed invalidly. */
        if (f->data.formals->count != 2)
        {
            return lval_err("Function format invalid. "
                            "Symbol '&' not followed by single symbol.");
        }

        /* Pop and delete '&' symbol */
        lval_del(lval_pop(f->data.formals, 0));

        /* Pop next symbol and create empty list */
        lval *sym = lval_pop(f->data.formals, 0);
        lval *val = lval_qexpr();

        /* Bind to environment and delete */
        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
    }

    /* If all formals have been bound evaluate */
    if (f->data.formals->count == 0)
    {

        /* Set environment parent to evaluation environment */
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(f->env,
                            lval_add(lval_sexpr(), lval_copy(f->data.body)));
    }
    else
    {
        /* Otherwise return partially evaluated function */
        return lval_copy(f);
    }
}

void lval_del(lval *v)
{
    switch (v->type)
    {
    case LVAL_FUN:
        if (!v->data.builtin)
        {
            lenv_del(v->env);
            lval_del(v->data.formals);
            lval_del(v->data.body);
        }
        else
        {
            free(v->func_name);
        }
        break;
    case LVAL_LONG:
    case LVAL_DOUBLE:
        break;
    case LVAL_ERR:
        free(v->data.err);
        break;
    case LVAL_SYM:
        free(v->data.sym);
        break;
    case LVAL_STR:
        free(v->data.str);
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

    if (e->par)
        return lenv_get(e->par, k);
    else
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

void lenv_def(lenv *e, lval *k, lval *v)
{
    while (e->par)
        e = e->par;
    lenv_put(e, k, v);
}

lval *builtin_add(lenv *e, lval *a) { return builtin_op_internal(e, a, "+"); }
lval *builtin_sub(lenv *e, lval *a) { return builtin_op_internal(e, a, "-"); }
lval *builtin_mul(lenv *e, lval *a) { return builtin_op_internal(e, a, "*"); }
lval *builtin_div(lenv *e, lval *a) { return builtin_op_internal(e, a, "/"); }
lval *builtin_pow(lenv *e, lval *a) { return builtin_op_internal(e, a, "^"); }
lval *builtin_mod(lenv *e, lval *a) { return builtin_op_internal(e, a, "%"); }
lval *builtin_min(lenv *e, lval *a) { return builtin_op_internal(e, a, "min"); }
lval *builtin_max(lenv *e, lval *a) { return builtin_op_internal(e, a, "max"); }

lval *builtin_def(lenv *e, lval *a) { return builtin_var(e, a, "def"); }
lval *builtin_put(lenv *e, lval *a) { return builtin_var(e, a, "="); }

lval *builtin_gt(lenv *e, lval *a) { return builtin_ord(e, a, ">"); }
lval *builtin_lt(lenv *e, lval *a) { return builtin_ord(e, a, "<"); }
lval *builtin_ge(lenv *e, lval *a) { return builtin_ord(e, a, ">="); }
lval *builtin_le(lenv *e, lval *a) { return builtin_ord(e, a, "<="); }

lval *builtin_eq(lenv *e, lval *a) { return builtin_cmp(e, a, "=="); }
lval *builtin_ne(lenv *e, lval *a) { return builtin_cmp(e, a, "!="); }
lval *builtin_or(lenv *e, lval *a) { return builtin_cmp(e, a, "||"); }
lval *builtin_and(lenv *e, lval *a) { return builtin_cmp(e, a, "&&"); }
lval *builtin_not(lenv *e, lval *a) { return builtin_cmp(e, a, "!"); }

int lval_eq(lval *x, lval *y)
{
    double xnum = (x->type == LVAL_DOUBLE) ? x->data.dnum : (double)x->data.num;
    double ynum = (y->type == LVAL_DOUBLE) ? y->data.dnum : (double)y->data.num;

    if (x->type != y->type)
        return 0;

    switch (x->type)
    {
    case LVAL_LONG:
    case LVAL_DOUBLE:
        return xnum == ynum;
    case LVAL_SYM:
        return (strcmp(x->data.sym, y->data.sym) == 0);
    case LVAL_STR:
        return (strcmp(x->data.str, y->data.str) == 0);
    case LVAL_ERR:
        return (strcmp(x->data.err, y->data.err) == 0);
    case LVAL_FUN:
        if (x->data.builtin || y->data.builtin)
            return x->data.builtin == y->data.builtin;
        else
            return lval_eq(x->data.formals, y->data.formals) && lval_eq(x->data.body, y->data.body);
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        if (x->count != y->count)
            return 0;
        for (int i = 0; i < x->count; i++)
        {
            if (!lval_eq(x->cell[i], y->cell[i]))
                return 0;
        }
        return 1;
        break;
    }
    return 0;
}

int lval_or(lval *x, lval *y)
{
    if (!x || !y)
        return 0;

    if (x->type != y->type)
        return 0;

    double xnum = (x->type == LVAL_DOUBLE) ? x->data.dnum : (double)x->data.num;
    double ynum = (y->type == LVAL_DOUBLE) ? y->data.dnum : (double)y->data.num;

    switch (x->type)
    {
    case LVAL_LONG:
    case LVAL_DOUBLE:
        return xnum || ynum;
    case LVAL_SYM:
        return strlen(x->data.sym) > 0 || strlen(y->data.sym) > 0;
    case LVAL_ERR:
        return strlen(x->data.err) > 0 || strlen(y->data.err);
    case LVAL_FUN:
        if (x->data.builtin || y->data.builtin)
            return 1;
        else
            return lval_or(x->data.formals, y->data.formals) || lval_or(x->data.body, y->data.body);
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        return (x->count > 0 || y->count > 0);
    }
    return 0;
}

int lval_and(lval *x, lval *y)
{
    if (!x || !y)
        return 0;

    if (x->type != y->type)
        return 0;

    double xnum = (x->type == LVAL_DOUBLE) ? x->data.dnum : (double)x->data.num;
    double ynum = (y->type == LVAL_DOUBLE) ? y->data.dnum : (double)y->data.num;

    switch (x->type)
    {
    case LVAL_LONG:
    case LVAL_DOUBLE:
        return xnum && ynum;
    case LVAL_SYM:
        return strlen(x->data.sym) > 0 && strlen(y->data.sym) > 0;
    case LVAL_ERR:
        return strlen(x->data.err) > 0 && strlen(y->data.err) > 0;
    case LVAL_FUN:
        if (x->data.builtin && y->data.builtin)
            return 1;
        else
            return lval_and(x->data.formals, y->data.formals) && lval_and(x->data.body, y->data.body);
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        return (x->count > 0 && y->count > 0);
    }
    return 0;
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func)
{
    lval *k = lval_sym(name);
    lval *v = lval_fun(func, name);
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
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "load", builtin_load);
    lenv_add_builtin(e, "error", builtin_error);
    lenv_add_builtin(e, "print", builtin_print);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "\\", builtin_lambda);
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);
    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "<=", builtin_le);
    lenv_add_builtin(e, "||", builtin_or);
    lenv_add_builtin(e, "&&", builtin_and);
    lenv_add_builtin(e, "!", builtin_not);

    // Arithmetic operators
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "^", builtin_pow);
    lenv_add_builtin(e, "^", builtin_pow);
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

lval *lval_str(char *s)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->data.str = malloc(strlen(s) + 1);
    strcpy(v->data.str, s);
    return v;
}

lval *lval_fun(lbuiltin func, const char *name)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->data.builtin = func;
    v->func_name = malloc(strlen(name) + 1);
    strcpy(v->func_name, name);
    return v;
}

lval *lval_lambda(lval *formals, lval *body)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    // Set Builtin to NULL
    v->data.builtin = NULL;

    v->env = lenv_new();
    v->data.formals = formals;
    v->data.body = body;

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
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lval_print_str(lval *v)
{
    char *escaped = malloc(strlen(v->data.str) + 1);
    strcpy(escaped, v->data.str);
    escaped = mpcf_escape(escaped);
    printf("\"%s\"", escaped);
    free(escaped);
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
    case LVAL_STR:
        lval_print_str(v);
        break;
    case LVAL_FUN:
        if (v->data.builtin)
            printf("<%s>", v->func_name);
        else
        {
            printf("(\\ )");
            lval_print(v->data.formals);
            putchar(' ');
            lval_print(v->data.body);
            putchar(')');
        }
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

int lval_is_true(lval *v)
{
    switch (v->type)
    {
    case LVAL_LONG:
        return v->data.num != 0;
    case LVAL_DOUBLE:
        return v->data.dnum != 0.0;
    case LVAL_ERR:
        return strlen(v->data.err) > 0;
    case LVAL_STR:
        return strlen(v->data.str) > 0;
    case LVAL_SYM:
        return strlen(v->data.sym) > 0;
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        return v->count > 0;
    default:
        return 1;
    }
}

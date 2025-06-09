#include <math.h>
#include <string.h>
#include "evaluation.h"
#include "error_handling.h"

lval eval_op(lval x, char *op, lval y)
{
    if (x.type == LVAL_ERR)
        return x;
    if (y.type == LVAL_ERR)
        return y;

    int is_double = (x.type == LVAL_DOUBLE || y.type == LVAL_DOUBLE);

    double xnum = (x.type == LVAL_DOUBLE) ? x.data.dnum : (double)x.data.num;
    double ynum = (y.type == LVAL_DOUBLE) ? y.data.dnum : (double)y.data.num;

    if (strcmp(op, "+") == 0)
    {
        if (is_double)
            return lval_double(xnum + ynum);
        else
            return lval_long(x.data.num + y.data.num);
    }
    if (strcmp(op, "-") == 0)
    {
        if (is_double)
            return lval_double(xnum - ynum);
        else
            return lval_long(x.data.num - y.data.num);
    }
    if (strcmp(op, "*") == 0)
    {
        if (is_double)
            return lval_double(xnum * ynum);
        else
            return lval_long(x.data.num * y.data.num);
    }
    if (strcmp(op, "/") == 0)
    {
        if (ynum == 0)
            return lval_err(LERR_DIV_ZERO);
        if (is_double)
            return lval_double(xnum / ynum);
        else
            return lval_long(x.data.num / y.data.num);
    }
    if (strcmp(op, "%") == 0)
    {
        if (is_double)
            return lval_err(LERR_BAD_OP);
        if (y.data.num == 0)
            return lval_err(LERR_DIV_ZERO);
        return lval_long(x.data.num % y.data.num);
    }
    if (strcmp(op, "^") == 0)
    {
        if (is_double)
            return lval_double(pow(xnum, ynum));
        if (y.data.num < 0)
            return lval_err(LERR_BAD_OP);

        long result = 1;
        while (y.data.num > 0)
        {
            if (y.data.num % 2 == 1)
            {
                result *= x.data.num;
            }
            x.data.num *= x.data.num;
            y.data.num /= 2;
        }
        return lval_long(result);
    }
    if (strcmp(op, "min") == 0)
    {
        if (is_double)
            return lval_double(fmin(xnum, ynum));
        return lval_long(y.data.num ^ ((x.data.num ^ y.data.num) & -(x.data.num < y.data.num)));
    }
    if (strcmp(op, "max") == 0)
    {
        if (is_double)
            return lval_double(fmax(xnum, ynum));
        return lval_long(x.data.num ^ ((x.data.num ^ y.data.num) & -(x.data.num < y.data.num)));
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
    if (strstr(t->tag, "number"))
    {
        errno = 0;
        if (strchr(t->contents, '.'))
        {
            double x = strtod(t->contents, NULL);
            return errno != ERANGE ? lval_double(x) : lval_err(LERR_BAD_NUM);
        }
        else
        {
            long x = strtol(t->contents, NULL, 10);
            return errno != ERANGE ? lval_long(x) : lval_err(LERR_BAD_NUM);
        }
    }

    char *op = t->children[1]->contents; // Operator is second child in case of expr
    lval x = eval(t->children[2]);       // Store the third child

    int i = 3;
    while (strstr(t->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}
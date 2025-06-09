#include <math.h>
#include "evaluation.h"
#include "error_handling.h"

lval eval_op(lval x, char *op, lval y)
{
    if (x.type == LVAL_ERR)
    {
        return x;
    }
    if (y.type == LVAL_ERR)
    {
        return y;
    }

    if (strcmp(op, "+") == 0)
    {
        return lval_num(x.data.num + y.data.num);
    }
    if (strcmp(op, "-") == 0)
    {
        return lval_num(x.data.num - y.data.num);
    }
    if (strcmp(op, "*") == 0)
    {
        return lval_num(x.data.num * y.data.num);
    }
    if (strcmp(op, "/") == 0)
    {
        return y.data.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.data.num / y.data.num);
    }
    if (strcmp(op, "%") == 0)
    {
        return lval_num(x.data.num % y.data.num);
    }
    if (strcmp(op, "^") == 0)
    {
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
        return lval_num(result);
    }
    if (strcmp(op, "min") == 0)
    {
        return lval_num(y.data.num ^ ((x.data.num ^ y.data.num) & -(x.data.num < y.data.num)));
    }
    if (strcmp(op, "max") == 0)
    {
        return lval_num(x.data.num ^ ((x.data.num ^ y.data.num) & -(x.data.num < y.data.num)));
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
    if (strstr(t->tag, "number"))
    {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
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
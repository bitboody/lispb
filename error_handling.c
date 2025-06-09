#include <stdio.h>
#include "error_handling.h"

lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.data.num = x;
    return v;
}

lval lval_double(double x)
{
    lval v;
    v.type = LVAL_DOUBLE;
    v.data.dnum = x;
    return v;
}

lval lval_err(int x)
{
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v)
{
    switch (v.type)
    {
    case LVAL_NUM:
        printf("%li", v.data.num);
        break;
    case LVAL_DOUBLE:
        printf("%lf", v.data.dnum);
        break;
    case LVAL_ERR:
        if (v.err == LERR_DIV_ZERO)
        {
            printf("Error: Division by Zero");
        }
        if (v.err == LERR_BAD_OP)
        {
            printf("Error: not a valid operator");
        }
        if (v.err == LERR_BAD_NUM)
        {
            printf("Error: Invalid number");
        }
        break;
    }
}

void lval_println(lval v)
{
    lval_print(v);
    putchar('\n');
}
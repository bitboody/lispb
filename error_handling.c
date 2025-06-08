#include <stdio.h>
#include "error_handling.h"

lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x, char *y)
{
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    v.s = y;
    return v;
}

void lval_print(lval v)
{
    switch (v.type)
    {
    case LVAL_NUM:
        printf("%li", v.num);
        break;
    case LVAL_ERR:
        if (v.err == LERR_DIV_ZERO)
        {
            printf("Error: Division by Zero");
        }
        if (v.err == LERR_BAD_OP)
        {
            printf("Error: %s is not a valid operator", v.s);
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
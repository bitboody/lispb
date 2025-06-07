#include "evaluation.h"

long eval_op(long x, char *op, long y)
{
    if (strcmp(op, "+") == 0)
    {
        return x + y;
    }
    if (strcmp(op, "-") == 0)
    {
        return x - y;
    }
    if (strcmp(op, "*") == 0)
    {
        return x * y;
    }
    if (strcmp(op, "/") == 0)
    {
        return x / y;
    }
    if (strcmp(op, "%") == 0)
    {
        return x % y;
    }
    return 0;
}

long eval(mpc_ast_t *t)
{
    if (strstr(t->tag, "number")) // if the tag of an ast is number
    {
        return atoi(t->contents); // convert the str to an int and return it
    }

    char *op = t->children[1]->contents; // Operator is second child in case of expr
    long x = eval(t->children[2]);       // Store the third child

    int i = 3;
    while (strstr(t->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}
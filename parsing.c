#include "lib/mpc.h"

long eval(mpc_ast_t *t);

int main()
{
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // defining the parsers
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                       \
        number   : /-?[0-9]+/ | <number>+('.'<number>)* ;   \
        operator : '+' | '-' | '*' | '/' | '%' ;            \
        expr     : <number> | '(' <operator> <expr>+ ')' ;  \
        lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
              Number, Operator, Expr, Lispy);

    char *prompt = "lispy> ";
    char buffer[2048];

    printf("Lispy Interpreter\n");
    printf("Press Ctrl+c to Exit\n");
    while (1)
    {
        fputs(prompt, stdout);
        fgets(buffer, 2048, stdin);

        mpc_result_t r;
        if (mpc_parse("<stdin>", buffer, Lispy, &r))
        {
            long result = eval(r.output);
            printf("%li\n", result);
            // mpc_ast_print(r.output);
            printf("%d\n", number_of_nodes(r.output));
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}

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
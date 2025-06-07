#include "lib/mpc.h"
#include "evaluation.h"

long eval(mpc_ast_t *t);

int main()
{
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // defining the parsers
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                             \
        number   : /-?[0-9]+/ | <number>+('.'<number>)* ;   \
        operator : '+' | '-' | '*' | '/' | '%' | '^' |      \
                    \"min\" | \"max\" ;                     \
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
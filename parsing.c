#include "lib/mpc.h"

int main()
{
    mpc_parser_t *Digit = mpc_new("digit");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // defining the parsers
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        digit    : /-?[0-9]+/ ;                             \
        number   : <digit>+('.'<digit>)* ;                  \
        operator : '+' | '-' | '*' | '/' | '%' ;            \
        expr     : <number> | '(' <operator> <expr>+ ')' ;  \
        lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Digit, Number, Operator, Expr, Lispy);

    char* prompt = "lispy> ";
    char buffer[2048];

    printf("Lispy Interpreter\n");
    printf("Press Ctrl+c to Exit\n");
    while (1)
    {
        fputs(prompt, stdout);
        fgets(buffer, 2048, stdin);
        
        mpc_result_t result;
        if (mpc_parse("<stdin>", buffer, Lispy, &result))
        {
            mpc_ast_print(result.output);
            mpc_ast_delete(result.output);
        } else {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }
    }

    mpc_cleanup(5, Digit, Number, Operator, Expr, Lispy);

    return 0;
}

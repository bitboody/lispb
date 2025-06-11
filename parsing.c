#include "lib/mpc.h"
#include "evaluation.h"
#include "error_handling.h"

#ifdef _WIN32
static char buffer[2048];

char *readline(char *prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

void add_history(char *unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main()
{
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *LispB = mpc_new("lispb");

    // defining the parsers
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                    \
        number   : /-?([0-9]+\\.[0-9]*|[0-9]*\\.[0-9]+|[0-9]+)/ ;  \
        symbol   : '+' | '-' | '*' | '/' | '%' | '^' |             \
                    \"min\" | \"max\" ;                            \
        sexpr    : '(' <expr>* ')' ;                               \
        expr     : <number> | <symbol> | <sexpr> ;                 \
        lispb    : /^/ <expr>* /$/ ;                               \
    ",
              Number, Symbol, Sexpr, Expr, LispB);

    printf("LispB Interpreter\n");
    printf("Press Ctrl+c to Exit\n\n");

    while (1)
    {
        char *input = readline("lispb> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, LispB, &r))
        {
            // lval result = eval(r.output);
            // lval_println(result);
            lval *x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);
            // mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, LispB);

    return 0;
}
#include "lib/mpc.h"
#include "include/lval.h"
#include "include/evaluation.h"

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

mpc_parser_t *Number;
mpc_parser_t *Symbol;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *LispB;

int main(int argc, char *argv[])
{
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    String = mpc_new("string");
    Comment = mpc_new("comment");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    Expr = mpc_new("expr");
    LispB = mpc_new("lispb");

    // defining the parsers
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                         \
        number   : /-?([0-9]+\\.[0-9]*|[0-9]*\\.[0-9]+|[0-9]+)/ ;       \
        symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\^=<>!&]+/ ;                  \
        string   : /\"(\\\\.|[^\"])*\"/ ;                               \
        comment  : /;[^\\r\\n]*/ ;                                      \
        sexpr    : '(' <expr>* ')' ;                                    \
        qexpr    : '{' <expr>* '}' ;                                    \
        expr     : <number> | <symbol> | <string>                       \
                 | <comment> | <sexpr> | <qexpr> ;                      \
        lispb    : /^/ <expr>* /$/ ;                                    \
    ",
              Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, LispB);

    lenv *e = lenv_new();
    lenv_add_builtins(e);

    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {
            lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));

            lval *x = builtin_load(e, args);

            if (x->type == LVAL_ERR)
                lval_println(x);
            lval_del(x);
        }
    }

    printf("LispB Interpreter\n");
    printf("Press Ctrl+c to Exit\n\n");

    while (1)
    {
        char *input = readline("lispb> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, LispB, &r))
        {
            lval *x = lval_eval(e, lval_read(r.output));
            if (x->count > 0)
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
        free(input);
    }
    lenv_del(e);
    mpc_cleanup(8, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, LispB);

    return 0;
}
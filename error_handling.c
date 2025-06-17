#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "include/lval.h"

lval *lval_err(char *fmt, ...)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    // Create a va list and initialize it
    va_list va;
    va_start(va, fmt);

    // Allocate 512 bytes
    v->data.err = malloc(512);

    // printf the error string with max 511 chars
    vsnprintf(v->data.err, 511, fmt, va);

    v->data.err = realloc(v->data.err, strlen(v->data.err) + 1);

    va_end(va);

    return v;
}

char *ltype_name(int t)
{
    switch (t)
    {
    case LVAL_FUN:
        return "Function";
    case LVAL_LONG:
        return "Long";
    case LVAL_DOUBLE:
        return "Double";
    case LVAL_ERR:
        return "Error";
    case LVAL_SYM:
        return "Symbol";
    case LVAL_SEXPR:
        return "S-Expression";
    case LVAL_QEXPR:
        return "Q-Expression";
    default:
        return "Unknown";
    }
}
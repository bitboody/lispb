#include <stdlib.h>
#include <string.h>
#include "include/lval.h"

lval *lval_err(char *m)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->data.err = malloc(strlen(m) + 1);
    strcpy(v->data.err, m);
    return v;
}
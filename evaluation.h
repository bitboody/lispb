#ifndef EVALUATION_H
#define EVALUATION_H

#include "lib/mpc.h"

long eval_op(long x, char *op, long y);
long eval(mpc_ast_t *t);

#endif
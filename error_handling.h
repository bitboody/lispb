#ifndef ERROR_HANDLING_H 
#define ERROR_HANDLING_H 
typedef struct
{
    int type;
    long num;
    int err;
    char *s;
} lval;

/* Possible lval types*/
enum
{
    LVAL_NUM,
    LVAL_ERR
};

/* Possible error types*/
enum
{
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

lval lval_num(long x);
lval lval_err(int x, char *y);
void lval_println(lval v);

#endif
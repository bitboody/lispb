# LispB 
A lisp interpreter using buildyourownlisp.com guidance

Build command:
```psh
gcc -std=c99 -Wall parsing.c lval.c evaluation.c error_handling.c ./lib/mpc.c -o lispb 
```

or use ```make``` if you have it installed.
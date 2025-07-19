# LispB 
A lisp interpreter using buildyourownlisp.com guidance

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/bitboody/lispb/c-cpp.yml) ![GitHub top language](https://img.shields.io/github/languages/top/bitboody/lispb)


Build command for windows:
```psh
gcc -std=c99 -Wall parsing.c lval.c evaluation.c error_handling.c ./lib/mpc.c -o lispb 
```

or use ```make windows``` if you have Make installed.
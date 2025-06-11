all: parser

parser: parsing.c
	gcc -std=c99 -Wall parsing.c lval.c evaluation.c error_handling.c ./lib/mpc.c -o lispb 
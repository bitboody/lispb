all: linux 

linux: parsing.c
	gcc -std=c99 -Wall parsing.c lval.c evaluation.c error_handling.c ./lib/mpc.c -ledit -lm -o ./bin/lispb

windows: parsing.c
	gcc -std=c99 -Wall parsing.c lval.c evaluation.c error_handling.c ./lib/mpc.c -o ./bin/lispb
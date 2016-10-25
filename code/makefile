all:	SETL genWorld SETL_par run

SETL:	SETL.c
	gcc -o SETL SETL.c

SETL_par: SETL_par.c
	mpicc -o SETL_par SETL_par.c

genWorld:	genWorld.c
	gcc -o genWorld genWorld.c

run: SETL_par
	mpirun -np 2 ./SETL_par ../data/simple.w 2 ../data/simple.p



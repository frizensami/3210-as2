all:	SETL genWorld SETL_par run

SETL:	SETL.c
	gcc -o SETL SETL.c

SETL_par: SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h
	mpicc -o SETL_par SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h

genWorld:	genWorld.c
	gcc -o genWorld genWorld.c

run: SETL_par
	mpirun -np 4 ./SETL_par ../data/simple.w 2 ../data/simple.p



all:	run

SETL:	SETL.c
	gcc -o SETL SETL.c

SETL_par: SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h
	mpicc -o SETL_par SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h

genWorld:	genWorld.c
	gcc -o genWorld genWorld.c

run: SETL_par
	mpirun -np 5 ./SETL_par ../data/simple.w 10 ../data/simple.p

runseq: SETL
	./SETL ../data/simple.w 10 ../data/simple.p

runlarge: SETL_par
	mpirun -np 2 ./SETL_par ../data/random1000.w 10 ../data/simple.p

runlargeseq: SETL
	./SETL ../data/random1000.w 10 ../data/simple.p


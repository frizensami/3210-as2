all:	run

compile: SETL SETL_par

SETL:	SETL.c
	gcc --std=c99 -o SETL SETL.c -lrt

SETL_par: SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h
	mpicc -O3 --std=c99 -o SETL_par SETL_par.c helper.h helper.c squarematrix.h squarematrix.c world.h world.c match.h match.c search.c search.h -lrt

genWorld:	genWorld.c
	gcc -o genWorld genWorld.c

run: SETL_par
	mpirun -np 2 ./SETL_par ../data/simple.w 10 ../data/simple.p

runlab: SETL_par
	mpirun -np 12 --machinefile machinefile.lab ./SETL_par ../data/random3000.w 100 ../data/glider5.p

runlabseq: SETL
	./SETL ../data/random3000.w 100 ../data/glider5.p
	
runlabsmall: SETL_par
	mpirun -np 6 ./SETL_par ../data/random3000.w 100 ../data/glider5.p

runlabseqsmall: SETL
	./SETL ../data/random3000.w 100 ../data/glider5.p

runlong: SETL_par
	mpirun -np 2 ./SETL_par ../data/simple.w 20 ../data/simple.p

run5: SETL_par
	mpirun -np 5 ./SETL_par ../data/simple.w 10 ../data/simple.p

run10: SETL_par
	mpirun -np 10 ./SETL_par ../data/simple.w 10 ../data/simple.p

runseq: SETL
	./SETL ../data/simple.w 10 ../data/simple.p

runlarge: SETL_par
	mpirun -np 5 ./SETL_par ../data/random1000.w 10 ../data/simple.p

runlargeseq: SETL
	./SETL ../data/random1000.w 10 ../data/simple.p

runlongseq: SETL
	./SETL ../data/simple.w 20 ../data/simple.p

runlargelong: SETL_par
	mpirun -np 10 ./SETL_par ../data/random1000.w 50 ../data/simple.p

runlargelongseq: SETL
	./SETL ../data/random1000.w 50 ../data/simple.p

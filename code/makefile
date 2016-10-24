all:	SETL genWorld

SETL:	SETL.c
	gcc -o SETL SETL.c

genWorld:	genWorld.c
	gcc -o genWorld genWorld.c

run: SETL genWorld
	./SETL ../data/simple.w 2 ../data/simple.p



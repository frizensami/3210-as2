#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"
#include "helper.h"
#include "world.h"
#include "squarematrix.h"
#include "match.h"
#include "search.h"

#define DEBUG
// #define DEBUGMORE



/**********************************************************
   MPI + Parallelism
**********************************************************/
#define ROOT_PROCESS 0

/***********************************************************
   Main function
***********************************************************/


int main( int argc, char** argv)
{
    char **curW, **nextW, **temp, dummy[20];
    char **patterns[4];
    int dir, iterations, iter;
    int size, patternSize;
    long long before, after;
    MATCHLIST*list;

    // Parallelism-related variables
    int numprocs, rank;

    if (argc < 4 ){
        fprintf(stderr,
            "Usage: %s <world file> <Iterations> <pattern file>\n", argv[0]);
        exit(1);
    }

    // MPI - Initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Print out MPI Information
    printf("===> Current process rank: %d out of %d\n", rank, numprocs);

    // Allocate the current world and the next world
    curW = readWorldFromFile(argv[1], &size);
    nextW = allocateSquareMatrix(size+2, DEAD);

    printf("World Size = %d\n", size);

    iterations = atoi(argv[2]);
    printf("Iterations = %d\n", iterations);

    // Reads the pattern file and rotates 3 times (for E, S, W), we already have N pattern from .p file
    patterns[N] = readPatternFromFile(argv[3], &patternSize);
    for (dir = E; dir <= W; dir++){
        patterns[dir] = allocateSquareMatrix(patternSize, DEAD);
        rotate90(patterns[dir-1], patterns[dir], patternSize);
    }
    printf("Pattern size = %d\n", patternSize);

#ifdef DEBUG
    printSquareMatrix(patterns[N], patternSize);
    printSquareMatrix(patterns[E], patternSize);
    printSquareMatrix(patterns[S], patternSize);
    printSquareMatrix(patterns[W], patternSize);
#endif
    //Start timer
    before = wallClockTime();

    //Actual work start

    // Creates a new MATCHLIST* with 0 elements inside and a NULL pointer to the next element
    list = newList();

    for (iter = 0; iter < iterations; iter++){

#ifdef DEBUG
        printf("World Iteration.%d\n", iter);
        printSquareMatrix(curW, size+2);
#endif

        // Search the current iteration
        searchPatterns( curW, size, iter, patterns, patternSize, list);

        // Generate next generation
        evolveWorld( curW, nextW, size );

        // Make the "current world" be the evolved nextW world
        // Reuse the previous current world space by making it the "next world"
        temp = curW;
        curW = nextW;
        nextW = temp;
    }

    // Prints the match list
    printList( list );

    //Stop timer
    after = wallClockTime();

    printf("Parallel SETL took %1.2f seconds\n",
        ((float)(after - before))/1000000000);


    //Clean up
    deleteList( list );

    freeSquareMatrix( curW );
    freeSquareMatrix( nextW );

    freeSquareMatrix( patterns[0] );
    freeSquareMatrix( patterns[1] );
    freeSquareMatrix( patterns[2] );
    freeSquareMatrix( patterns[3] );

    // Finalize MPI
    MPI_Finalize();

    return 0;
}







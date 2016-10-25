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

    // Check for correct number of arguments
    if (argc < 4){
        fprintf(stderr,"Usage: %s <world file> <Iterations> <pattern file>\n", argv[0]);
        exit(1);
    }

    /*************************
        Parse arguments
    **************************/
    iterations = atoi(argv[2]);

    /*************************
        MPI
    *************************/

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numprocs != 2 && numprocs != 3) {
        printf("Incorrect number of processors, got %d: only 2 or 3 accepted for now\n", numprocs);
        exit(1);
    }

    // Print out MPI Information
    // printf("===> Current process rank: %d out of %d\n", rank, numprocs);

    if (rank == ROOT_PROCESS) {
        printf("I am the root process! Starting transfer.\n");

        /*************************
            File operations
        **************************/
        patterns[N] = readPatternFromFile(argv[3], &patternSize);
        curW = readWorldFromFile(argv[1], &size);

        /*************************
            Memory allocations
            and initializations
        **************************/
        nextW = allocateSquareMatrix(size+2, DEAD);

        // Rotates pattern 3 times (for E, S, W), we already have N pattern from .p file
        for (dir = E; dir <= W; dir++){
            patterns[dir] = allocateSquareMatrix(patternSize, DEAD);
            rotate90(patterns[dir-1], patterns[dir], patternSize);
        }

        // Creates a new MATCHLIST* with 0 elements inside and a NULL pointer to the next element
        list = newList();

        /**************************
            Debug printing
        **************************/
        printf("World Size = %d\n", size);
        printf("Iterations = %d\n", iterations);
        printf("Pattern size = %d\n", patternSize);
        printf("Patterns (N, E, S, W): \n");

#ifdef DEBUG
        printSquareMatrix(patterns[N], patternSize);
        printSquareMatrix(patterns[E], patternSize);
        printSquareMatrix(patterns[S], patternSize);
        printSquareMatrix(patterns[W], patternSize);
        printf("Original Board: \n");
        printSquareMatrix(curW, size + 2);
#endif


    } else {
        printf("===> Not a root process, rank: %d out of %d\n", rank, numprocs);
    }


    // Start timer
    before = wallClockTime();

    // Actual work start

    /*************************
        DO COMMUNICATION
    *************************/

    // Send sizes over to be used throughout
    MPI_Bcast(&patternSize, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    MPI_Bcast(&size, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    MPI_Bcast(&iterations, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);

    printf("Process: %d - Patternsize: %d, Size: %d, Iterations: %d\n", rank, patternSize, size, iterations);

    // For all non-root processes - need to allocate arrays
    if (rank != ROOT_PROCESS) {
        // allocate space for the current board
        curW = allocateSquareMatrix(size + 2, DEAD);
        nextW = allocateSquareMatrix(size+2, DEAD);

        // allocate space for the patterns
        for (dir = N; dir <= W; dir++){
            patterns[dir] = allocateSquareMatrix(patternSize, DEAD);
        }
    }

    // Broadcast all patterns
    for (dir = N; dir <= W; dir++) {
        MPI_Bcast(*patterns[dir], patternSize * patternSize, MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);
    }

    // For this algo iter: broadcast the entire board and patterns
    MPI_Bcast(curW[0], (size + 2) * (size + 2), MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);

    // At this point: Every process has all the relevant data to perform computations
    // Processes just must know what their role is in the overall scheme of things
    // Processor 0: Always does pattern searching
    // Processor 1: Does evolution (part 1 or all)
    // Processor 2: Does evolution (part 2)

    // All processors iterate
    for (iter = 0; iter < iterations; iter++) {
        if (rank == ROOT_PROCESS) {
            // Definitely doing pattern searching

            // Search the current iteration
            searchPatterns( curW, size, iter, patterns, patternSize, list);

            MPI_Status status;
            // Receive the new world from the other processor
            MPI_Recv(curW[0], (size + 2) * (size + 2), MPI_CHAR, 1, 1, MPI_COMM_WORLD, &status);

        } else if (rank == 1) {

            // evolves the current world
            evolveWorld( curW, nextW, size );

            // Make the "current world" be the evolved nextW world
            // Reuse the previous current world space by making it the "next world"
            temp = curW;
            curW = nextW;
            nextW = temp;

            // Send the evolved world back
            MPI_Send(curW[0], (size + 2) * (size + 2), MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        }
    }

    if (rank == ROOT_PROCESS) {

        // Prints the match list
        printList( list );

        //Stop timer
        after = wallClockTime();

        printf("Parallel SETL took %1.2f seconds\n",
            ((float)(after - before))/1000000000);

    }

/*
    for (iter = 0; iter < iterations; iter++) {

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
*/

    /*************************
        Clean up
    *************************/
    /*
    deleteList( list );

    freeSquareMatrix( curW );
    freeSquareMatrix( nextW );

    freeSquareMatrix( patterns[0] );
    freeSquareMatrix( patterns[1] );
    freeSquareMatrix( patterns[2] );
    freeSquareMatrix( patterns[3] );
    */
    // Finalize MPI
    MPI_Finalize();

    return 0;
}







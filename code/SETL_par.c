#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"
#include "helper.h"
#include "world.h"
#include "squarematrix.h"
#include "match.h"
#include "search.h"

//#define DEBUG
// #define DEBUGMORE

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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

    // MPI related
    int tag = 240;

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


    // Print out MPI Information
    // printf("===> Current process rank: %d out of %d\n", rank, numprocs);

    if (rank == ROOT_PROCESS) {
        //printf("I am the root process! Starting transfer.\n");

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
        //printf("World Size = %d\n", size);
        //printf("Iterations = %d\n", iterations);
        //printf("Pattern size = %d\n", patternSize);
        //printf("Patterns (N, E, S, W): \n");

        if (size % numprocs != 0 ) {
            printf("Incorrect number of processors, got %d: must evenly divide size %d\n", numprocs, size);
            exit(1);
        }

#ifdef DEBUG
        printSquareMatrix(patterns[N], patternSize);
        printSquareMatrix(patterns[E], patternSize);
        printSquareMatrix(patterns[S], patternSize);
        printSquareMatrix(patterns[W], patternSize);
        printf("Original Board: \n");
        printSquareMatrix(curW, size + 2);
#endif

        //printf("Effective size: %d. Size with padding: %d\n", size, size+2);
        //printf("Each processor must effectively take: %d rows", size / numprocs);
       //printf("But with padding in front and behind, each must take: %d rows\n", size / numprocs + 2);


    } else {
        //printf("===> Not a root process, rank: %d out of %d\n", rank, numprocs);
    }


    // Start timer
    before = wallClockTime();

    // Actual work start

    /*
        ALGORITHM

        Overall idea:
        - each processor gets some number of rows as data (rmb test is 3000 rows)
        - each processor will have some k number of rows to work on and some k' rows of extra
        that extends below its current rows for pattern searching
        - each processor gets all the patterns (max size 5)
        - each processor searches for patterns in the rows
        - each processor evolves the rows it has
        - each processors sends back 1) The patterns found + 2) The evolved rows
    */




    /*************************
        DO COMMUNICATION
    *************************/

    // Send sizes over to be used throughout
    MPI_Bcast(&patternSize, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    MPI_Bcast(&size, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    MPI_Bcast(&iterations, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);

    //printf("Process: %d - Patternsize: %d, Size: %d, Iterations: %d\n", rank, patternSize, size, iterations);

    // For all non-root processes - need to allocate arrays
    if (rank != ROOT_PROCESS) {
        // allocate space for the current board
        curW = allocateSquareMatrix(size+2, DEAD);
        nextW = allocateSquareMatrix(size+2, DEAD);

        // allocate space for the patterns
        for (dir = N; dir <= W; dir++){
            patterns[dir] = allocateSquareMatrix(patternSize, DEAD);
        }

        // Creates a new MATCHLIST* with 0 elements inside and a NULL pointer to the next element
        list = newList();
    }

    // Broadcast all patterns
    for (dir = N; dir <= W; dir++) {
        MPI_Bcast(*patterns[dir], patternSize * patternSize, MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);
    }

    // For this algo iter: broadcast the entire board and patterns
    MPI_Bcast(curW[0], (size + 2) * (size + 2), MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);

    // Assume even division of rows (this calculation is without accounting for the 'halo' method)
    int numRowsPerProcessor = size / numprocs;

    if (numRowsPerProcessor < patternSize) {
        printf("Insufficient rows for rank: %d - exiting\n", rank);
        exit(0);
    }

    // Calculate start and end rows
    int startRow = rank * numRowsPerProcessor;
    int endRow = startRow + numRowsPerProcessor - 1;

    // Since we have a one row + one column boundary -> we need to shift everyone one row down
    // Note that the current halo method assumes that we passed the search functions pointers
    // that INCLUDE the padding rows - so we need to pass it from the padding row onwards
    // So startrow doesn't change (actually now it does because we pass it to search)
    startRow += 1;
    endRow += 1;

    // However - we need to account for the pattern size: so we need to add enough rows such
    // that we can start from the last row that we want to compute and find a pattern that
    // many rows down: need to limit by the max size of the world though
    endRow += patternSize - 1;
    endRow = MIN(endRow, size);

    // After evolving - send your version of the world back to master
    // Each one sends their start row till end row
    // For all processors - we need to send the same amount of data
    int rowToSendStart = startRow;
    int rowToSendEnd = startRow + numRowsPerProcessor - 1;
    int elementsToSend = (rowToSendEnd - rowToSendStart + 1) * (size + 2);

    MPI_Barrier(MPI_COMM_WORLD);

    // Search for patterns in these rows
    // Increment the world pointers by the required amounts
    // size + 2 == number of elements in 1 row
    // startRow = how many rows there are before us (row 1 = 1 row of skipped elements)
    //char* rowStart = (*curW) + (startRow * (size + 2));
    //char* nextWRowStart = (*nextW) + (startRow * (size + 2));
    //printf("Startrow: %d. Sizeof(char): %d, size + 2: %d\n", startRow, sizeof(char), size+2);

    //printf("Processor rank %d doing rows %d - %d inclusive out of halo-ed size %d\n", rank, startRow, endRow, size+2);
    // Rows = numRowsPerProcessor, Size = Cols
    //printf("Processor rank %d: searchPatternsNonSquare(&rowStart: %p, numRowsPerProcessor: %d, size: %d, iter: %d, patterns: %p, patternSize: %d, list: %p\n", rank, &rowStart, numRowsPerProcessor, size, iter, patterns, patternSize, list);


    // All processors iterate
    for (iter = 0; iter < iterations; iter++) {
        // Every processor has all info. Need to decide what rows to take
        //searchPatternsNonSquare( &rowStart, numRowsPerProcessor, size, iter, patterns, patternSize, list);
        //printf("Rank %d search %d - %d\n", rank, startRow, endRow);
        searchPatternsNonSquare(curW, startRow, endRow, size, iter, patterns, patternSize, list);

        //printf("Rank %d finished searching patterns, printing list\n", rank);
        //printList( list );
        //printf("Rank %d finished printing list\n", rank);

        //printf("Rank %d starting to evolve world @ iteration %d\n", rank, iter);
        evolveWorldNonSquare(curW, startRow, nextW, startRow + numRowsPerProcessor - 1, size);
        //printf("Rank %d done evolving world: World = \n", rank);

        MPI_Barrier(MPI_COMM_WORLD);

        // After evolving - send your version of the world back to master
        // Each one sends their start row till end row
        // For all processors - we need to send the same amount of data
        //printf("Rank %d - rowToSendStart: %d rowToSendEnd %d elementsToSend: %d\n", rank, rowToSendStart, rowToSendEnd, elementsToSend);

        // Put into curW[1] because we want to avoid the 0th row
        MPI_Allgather(nextW[rowToSendStart], elementsToSend, MPI_CHAR, curW[1], elementsToSend, MPI_CHAR, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

    }

    for (int proc = 1; proc < numprocs; proc++) {
        if (rank == proc) {
            // If we are the process to be sending the data:


            //printf("Rank %d starting to evolve world at address: %p\n", rank, nextWRowStart);
            //evolveWorldNonSquare(curW, startRow, nextW, startRow + numRowsPerProcessor - 1, size);
            //printf("Rank %d printing final list: \n", rank);
            //printList(list);

            //printf("Rank %d converting list to array, list: \n", rank);
            int* matchArray = matchlistToArray(list);
            //printMatchArray(matchArray, list->nItem);

            int matchArraySize = list->nItem;

            //printf("Rank %d sending\n", rank);
            MPI_Send(&matchArraySize, 1, MPI_INT, ROOT_PROCESS, tag, MPI_COMM_WORLD);

            if (matchArraySize > 0) {
                int numberOfInts = matchArraySize * 4;
                MPI_Send(matchArray, numberOfInts, MPI_INT, ROOT_PROCESS, tag, MPI_COMM_WORLD);
            }

            //evolveWorld(curW, nextW, size);
            //printf("Rank %d done evolving world expected: World = \n", rank);
            //printSquareMatrix(nextW, size+2);

        } else if (rank == ROOT_PROCESS) {
            // If we are the root and we should be receiving

            int matchArraySize = 0;
            MPI_Status status;

            //printf("Root receiving from rank %d\n", proc);
            MPI_Recv(&matchArraySize, 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &status);
            //printf("Root completed match array size reception from rank %d\n", proc);

            if (matchArraySize > 0) {
                int numberOfInts = matchArraySize * 4;
                int* matchArrayRecv = malloc(numberOfInts * sizeof(int));

                //printf("Match array recv: %p\n", matchArrayRecv);

                //printf("Root receiving <<MATCHARRAY>> from rank %d\n", proc);
                MPI_Recv(matchArrayRecv, numberOfInts, MPI_INT, proc, tag, MPI_COMM_WORLD, &status);
                //printf("Root completed match array ACTUAL reception from rank %d\n", proc);

                for (int i = 0; i < numberOfInts; i+=4) {
                    //printf("match array recv: %p\n", matchArrayRecv);
                    //printf("i = %d, num = %d\n", i, numberOfInts);
                    //printf("Iter: %d", matchArrayRecv[i]);
                    //printf(" Row: %d", matchArrayRecv[i+1]);
                    //printf(" Column: %d", matchArrayRecv[i+2]);
                    //printf(" Pattern: %d\n", matchArrayRecv[i+3]);
                    insertEnd(list, matchArrayRecv[i], matchArrayRecv[i+1], matchArrayRecv[i+2], matchArrayRecv[i+3]);

                }

                free( matchArrayRecv );
            }
        }
    }


    /************************
        PRINTING AT END
    ************************/


    if (rank == ROOT_PROCESS) {

        //printf("\nFINAL LIST: \n");
        // Prints the match list
        //printList( list );

        MATCH* matches = matchlistToMatchArray(list);

        //printf("Printing match struct array: \n");
        //printMatchStructArray(matches, list->nItem);

        //printf("Sorting match struct array: \n");
        qsort(matches, list->nItem, sizeof(MATCH), matchSortFunc);
        printMatchStructArray(matches, list->nItem);

        //Stop timer
        after = wallClockTime();

        printf("Parallel SETL took %1.2f seconds\n",
            ((float)(after - before))/1000000000);

    }



    fflush(stdin);
    fflush(stdout);

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







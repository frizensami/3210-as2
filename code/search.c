#include <stdio.h>
#include <stdlib.h>

#include "squarematrix.h"
#include "search.h"
#include "world.h"
#include "match.h"
#include "helper.h"

/***********************************************************
   Search related functions
***********************************************************/

char** readPatternFromFile( char* fname, int* sizePtr )
{
    FILE* inf;

    char temp, **pattern;
    int i, j;
    int size;

    inf = fopen(fname,"r");
    if (inf == NULL)
        die(__LINE__);


    fscanf(inf, "%d", &size);
    fscanf(inf, "%c", &temp);

    pattern = allocateSquareMatrix( size, DEAD );

    for (i = 0; i < size; i++){
        for (j = 0; j < size; j++){
            fscanf(inf, "%c", &pattern[i][j]);
        }
        fscanf(inf, "%c", &temp);
    }

    *sizePtr = size;    //return size
    return pattern;
}


void rotate90(char** current, char** rotated, int size)
{
    int i, j;

    for (i = 0; i < size; i++){
        for (j = 0; j < size; j++){
            rotated[j][size-i-1] = current[i][j];
        }
    }
}

// For each direction - sets off a searchSinglePattern on that direction
void searchPatterns(char** world, int wSize, int iteration,
        char** patterns[4], int pSize, MATCHLIST* list)
{
    int dir;

    for (dir = N; dir <= W; dir++){
        searchSinglePattern(world, wSize, iteration,
                patterns[dir], pSize, dir, list);
    }

}

void searchSinglePattern(char** world, int wSize, int iteration,
        char** pattern, int pSize, int rotation, MATCHLIST* list)
{
    int wRow, wCol, pRow, pCol, match;


    for (wRow = 1; wRow <= (wSize-pSize+1); wRow++){
        for (wCol = 1; wCol <= (wSize-pSize+1); wCol++){
            match = 1;
#ifdef DEBUGMORE
            printf("Searching:(%d, %d)\n", wRow-1, wCol-1);
#endif
            for (pRow = 0; match && pRow < pSize; pRow++){
                for (pCol = 0; match && pCol < pSize; pCol++){
                    if(world[wRow+pRow][wCol+pCol] != pattern[pRow][pCol]){
#ifdef DEBUGMORE
                        printf("\tFailed:(%d, %d) %c != %c\n", pRow, pCol,
                            world[wRow+pRow][wCol+pCol], pattern[pRow][pCol]);
#endif
                        match = 0;
                    }
                }
            }
            if (match){
                insertEnd(list, iteration, wRow-1, wCol-1, rotation);
#ifdef DEBUGMORE
printf("*** FOUND PATTERN @ Row = %d, Col = %d\n", wRow-1, wCol-1);
#endif
            }
        }
    }
}

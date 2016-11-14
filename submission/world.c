#include <stdio.h>
#include <stdlib.h>

#include "world.h"
#include "helper.h"
#include "squarematrix.h"

/***********************************************************
   World  related functions
***********************************************************/

char** readWorldFromFile( char* fname, int* sizePtr )
{
    FILE* inf;

    char temp, **world;
    int i, j;
    int size;

    inf = fopen(fname,"r");
    if (inf == NULL)
        die(__LINE__);


    fscanf(inf, "%d", &size);
    fscanf(inf, "%c", &temp);

    //Using the "halo" approach
    // allocated additional top + bottom rows
    // and leftmost and rightmost rows to form a boundary
    // to simplify computation of cell along edges
    world = allocateSquareMatrix( size + 2, DEAD );

    for (i = 1; i <= size; i++){
        for (j = 1; j <= size; j++){
            fscanf(inf, "%c", &world[i][j]);
        }
        fscanf(inf, "%c", &temp);
    }

    *sizePtr = size;    //return size
    return world;

}

int countNeighbours(char** world, int row, int col)
//Assume 1 <= row, col <= size, no check
{
    int i, j, count;

    count = 0;
    for(i = row-1; i <= row+1; i++){
        for(j = col-1; j <= col+1; j++){
            count += (world[i][j] == ALIVE );
        }
    }

    //discount the center
    count -= (world[row][col] == ALIVE);

    return count;

}

void evolveWorld(char** curWorld, char** nextWorld, int size)
{
    int i, j, liveNeighbours;

    for (i = 1; i <= size; i++){
        for (j = 1; j <= size; j++){
            liveNeighbours = countNeighbours(curWorld, i, j);
            nextWorld[i][j] = DEAD;

            //Only take care of alive cases
            if (curWorld[i][j] == ALIVE) {

                if (liveNeighbours == 2 || liveNeighbours == 3)
                    nextWorld[i][j] = ALIVE;

            } else if (liveNeighbours == 3)
                    nextWorld[i][j] = ALIVE;
        }
    }
}


void evolveWorldNonSquare(char** curWorld, int startRow, char** nextWorld, int endRow, int wCols)
{
    // Note: The scanning will occur inclusive of curWorld[startRow - 1]
    int i, j, liveNeighbours;
    //printf("Evolving non square world: startrow = %d, endRow = %d, wCols = %d\n", startRow, endRow, wCols);

    for (i = startRow; i <= endRow; i++){
        for (j = 1; j <= wCols; j++){
            liveNeighbours = countNeighbours(curWorld, i, j);
            nextWorld[i][j] = DEAD;


            //Only take care of alive cases
            if (curWorld[i][j] == ALIVE) {

                if (liveNeighbours == 2 || liveNeighbours == 3)
                {

                    //printf("i: %d, j: %d, live: %d\n", i, j, liveNeighbours);
                    nextWorld[i][j] = ALIVE;
                }

            } else if (liveNeighbours == 3)
            {

                    //printf("i: %d, j: %d, live: %d\n", i, j, liveNeighbours);
                    nextWorld[i][j] = ALIVE;
            }
        }
    }
}

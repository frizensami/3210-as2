#ifndef WORLD_H
#define WORLD_H

/***********************************************************
   World  related functions
***********************************************************/

#define ALIVE 'X'
#define DEAD 'O'

char** readWorldFromFile( char* fname, int* size );

int countNeighbours(char** world, int row, int col);

void evolveWorld(char** curWorld, char** nextWorld, int size);

void evolveWorldNonSquare(char** curWorld, int startRow, char** nextWorld, int wRows, int wCols);

#endif

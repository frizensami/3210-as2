#ifndef SEARCH_H
#define SEARCH_H

/***********************************************************
   Search related functions
***********************************************************/
#include "match.h"

//Using the compass direction to indicate the rotation of pattern
#define N 0 //no rotation
#define E 1 //90 degree clockwise
#define S 2 //180 degree clockwise
#define W 3 //90 degree anti-clockwise

char** readPatternFromFile( char* fname, int* size );

void rotate90(char** current, char** rotated, int size);

void searchPatterns(char** world, int wSize, int iteration,
        char** patterns[4], int pSize, MATCHLIST* list);

void searchSinglePattern(char** world, int wSize, int interation,
        char** pattern, int pSize, int rotation, MATCHLIST* list);

void searchPatternsNonSquare(char** world, int startRow, int endRow, int wCols, int iteration,
        char** patterns[4], int pSize, MATCHLIST* list);

void searchSinglePatternNonSquare(char** world, int startRow, int endRow, int wCols, int interation,
        char** pattern, int pSize, int rotation, MATCHLIST* list);
#endif

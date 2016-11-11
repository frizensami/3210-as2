#ifndef MATCH_H
#define MATCH_H

/***********************************************************
   Simple circular linked list for match records
***********************************************************/
typedef struct MSTRUCT {
    int iteration, row, col, rotation;
    struct MSTRUCT *next;
} MATCH;


typedef struct {
    int nItem;
    MATCH* tail;
} MATCHLIST;

int* matchlistToArray( MATCHLIST* list );

MATCHLIST* newList();

void deleteList( MATCHLIST*);

void insertEnd(MATCHLIST*, int, int, int, int);

void printList(MATCHLIST*);

void printMatchArray(int*, int);

void insertSorted(MATCHLIST*, int*, int);

#endif

#include <stdlib.h>
#include <stdio.h>

#include "match.h"
#include "helper.h"
/***********************************************************
   Simple circular linked list for match records
***********************************************************/

// Convert a matchlist to a contiguous array of ints
int* matchlistToArray( MATCHLIST* list ) {
    int n = list->nItem;
    int* matchArray = malloc(4 * (list->nItem) * sizeof(int) );

    MATCH* current = list->tail;
    for (int i = 0; i < n*4; i+=4) {
        matchArray[i] = current->iteration;
        matchArray[i+1] = current->row;
        matchArray[i+2] = current->col;
        matchArray[i+3] = current->rotation;
        current = current->next;
    }

    return matchArray;
}

MATCHLIST* newList()
{
    MATCHLIST* list;

    list = (MATCHLIST*) malloc(sizeof(MATCHLIST));
    if (list == NULL)
        die(__LINE__);

    list->nItem = 0;
    list->tail = NULL;

    return list;
}

void deleteList( MATCHLIST* list)
{
    MATCH *cur, *next;
    int i;
    //delete items first

    if (list->nItem != 0 ){
        cur = list->tail->next;
        next = cur->next;
        for( i = 0; i < list->nItem; i++, cur = next, next = next->next ) {
            free(cur);
        }

    }
    free( list );
}

void insertEnd(MATCHLIST* list,
        int iteration, int row, int col, int rotation)
{
    MATCH* newItem;

    newItem = (MATCH*) malloc(sizeof(MATCH));
    if (newItem == NULL)
        die(__LINE__);

    newItem->iteration = iteration;
    newItem->row = row;
    newItem->col = col;
    newItem->rotation = rotation;

    if (list->nItem == 0){
        newItem->next = newItem;
        list->tail = newItem;
    } else {
        newItem->next = list->tail->next;
        list->tail->next = newItem;
        list->tail = newItem;
    }

    (list->nItem)++;

}

void printList(MATCHLIST* list)
{
    int i;
    MATCH* cur;

    printf("List size = %d\n", list->nItem);


    if (list->nItem == 0) return;

    cur = list->tail->next;
    for( i = 0; i < list->nItem; i++, cur=cur->next){
        printf("%d:%d:%d:%d\n",
                cur->iteration, cur->row, cur->col, cur->rotation);
    }
}

void printMatchArray(int* matchArray, int elements) {
    for (int i = 0; i < elements*4; i+=4) {
        printf("%d:%d:%d:%d\n", matchArray[i], matchArray[i+1], matchArray[i+2], matchArray[i+3]);
    }
}

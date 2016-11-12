/***********************************************************
  Helper functions
***********************************************************/

#define _POSIX_C_SOURCE 199309L
struct timespec time1, time2;

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
void die(int lineNo)
{
    fprintf(stderr, "Error at line %d. Exiting\n", lineNo);
    exit(1);
}


long long wallClockTime( )
{
#ifdef __linux__
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <unistd.h>

void *myMalloc(size_t size);
void myFree( void *ptr);
void *myCalloc(size_t nmemb, size_t size);
void *myRealloc(void *ptr, size_t size);

#endif //MEMALLOC_H

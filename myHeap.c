// COMP1521 18s1 Assignment 2
// Implementation of heap management system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myHeap.h"

// minimum total space for heap
#define MIN_HEAP  4096
// minimum amount of space for a free Chunk (excludes Header)
#define MIN_CHUNK 32

#define ALLOC     0x55555555
#define FREE      0xAAAAAAAA

typedef unsigned int uint;   // counters, bit-strings, ...

typedef void *Addr;          // addresses

typedef struct {             // headers for Chunks
    uint  status;             // status (ALLOC or FREE)
    uint  size;               // #bytes, including header
} Header;

static Addr  heapMem;        // space allocated for Heap
static int   heapSize;       // number of bytes in heapMem
static Addr *freeList;       // array of pointers to free chunks
static int   freeElems;      // number of elements in freeList[]
static int   nFree;          // number of free chunks
// LOOK AT DUMPHEAP TO SEE HOW USE AND GET STUFF, LOOPING THORUGH HEAP ETC

// initialise heap
int initHeap(int size)
{
	if (size < MIN_HEAP) size = MIN_HEAP;
	if (size % 4 != 0) size += 
    return 0; 
}

static void

// allocate a chunk of memory
void *myMalloc(int size)
{

    return NULL; // this just keeps the compiler quiet
}

// free a chunk of memory
void myFree(void *block)
{

}

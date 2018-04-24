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

static void roundUpToMultFour(int *value) {
	while (*value % 4 != 0) {
		(*value)++;
	}
}

// initialise heap
int initHeap(int size)
{
	// set heapSize
	if (size < MIN_HEAP) size = MIN_HEAP;
	if (size % 4 != 0) roundUpToMultFour(&size);
	heapSize = size;

	// allocate region of memory and sets heapMem to the first byte
	Addr heap = malloc(size);
	if (heap == NULL) {
		perror("Out of memory");
		return -1;
	}
	heapMem = heap;
	memset(heapMem, 0, size);

	// initialise region to a free chunk
	Header *chunk = (Header *)heapMem;
	chunk->status = FREE;
	chunk->size = size;

	// initialise freeList array and set first element to the single free chunk
	size /= MIN_CHUNK; 
	freeList = malloc(size*sizeof(Addr));
	if (freeList == NULL) {
		perror("Out of memory");
		return -1;
	}
	freeList[0] = (Addr)((char *)chunk);
	nFree = 1;
	freeElems = --size;

    return 0; 
}

// allocate a chunk of memory
void *myMalloc(int size)
{

    return NULL; // this just keeps the compiler quiet
}

// free a chunk of memory
void myFree(void *block)
{

}

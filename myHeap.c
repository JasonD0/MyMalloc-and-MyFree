// COMP1521 18s1 Assignment 2
// Implementation of heap management system
// Written By Jason Do

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

// size padding to multiple of four
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
	memset(heap, 0, size);
	heapMem = heap;

	// initialise region to a free chunk
	Header *chunk = (Header *)heapMem;
	chunk->status = FREE;
	chunk->size = size;             

	// initialise freeList array and point first element to the free chunk
	size /= MIN_CHUNK; 
	freeList = malloc(size*sizeof(Addr));
	if (freeList == NULL) {
		perror("Out of memory");
		return -1;
	}
	freeList[0] = (Addr)((char *)chunk);
	nFree = 1;
	freeElems = size;
	
    return 0; 
}

// clean heap
void freeHeap()
{
   free(heapMem);
   free(freeList);
}

// allocate a chunk of memory
void *myMalloc(int size)
{
	if (nFree == 0 || size < 1 || size > heapSize) return NULL;
    
    // size padding to multiple four
    if (size % 4 != 0) roundUpToMultFour(&size);
    
	Header *chunk = NULL;
	Header *smallestFreeChunk = NULL;
	int smallestChunkIndex = 0; 
	
	// find smallest free chunk
	for (int i = 0; i < nFree; i++) {
		chunk = (Header *)freeList[i];
		
		// get first free chunk with enough space
		if (smallestFreeChunk == NULL && chunk->size > size + sizeof(Header)) {
			smallestChunkIndex = i;
			smallestFreeChunk = (Header *)freeList[i];
			
		// skip next else if 
		} else if (smallestFreeChunk == NULL) {

		// sets current chunk to be the smallest chunk if enough space 	
		} else if (smallestFreeChunk->size > chunk->size && 
		           chunk->size > size + sizeof(Header)) {
			smallestFreeChunk = (Header *)freeList[i];	
			smallestChunkIndex = i;
		}
	}	
	
	// no free chunk large enough
	if (smallestFreeChunk == NULL) return NULL;

	// allocate whole chunk if chunk <= given size + headerSize + MIN_CHUNK 
	if (smallestFreeChunk->size <= size + sizeof(Header) + MIN_CHUNK) {     // spec says smaller than (?) 
		smallestFreeChunk->status = ALLOC;
		// remove smallest chunk from free list
		for (int i = smallestChunkIndex; i < nFree; i++) {
			freeList[i] = freeList[i+1];
		}
		nFree--;
		
	// split chunk into two parts if chunk > given size + headerSize + MIN_CHUNK
	} else if (smallestFreeChunk->size > size + sizeof(Header) + MIN_CHUNK) {
		// allocate lower chunk with given size 
		int oldSize = smallestFreeChunk->size;
		smallestFreeChunk->status = ALLOC;
		smallestFreeChunk->size = size + sizeof(Header);

		// create upper chunk and move pointer from old address to new address
		Addr lowerAddr = (Addr)((char *)freeList[smallestChunkIndex] + smallestFreeChunk->size); 
		chunk = (Header *)lowerAddr;  
		chunk->status = FREE;
		chunk->size = oldSize - smallestFreeChunk->size;
		freeList[smallestChunkIndex] = (Addr)((char *)freeList[smallestChunkIndex] + smallestFreeChunk->size);   
	}

    return (char *)smallestFreeChunk + sizeof(Header); 
}

// free a chunk of memory
void myFree(void *block)
{
	if (freeElems == nFree) return; 

	// check if block allocated chunk within the heap
	Addr heapTop = (Addr)((char *)heapMem + heapSize);
	if (block == NULL || block < heapMem || block >= heapTop) {		
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}
	
    // assumption : myFree used for myMalloc
    // get start address of block
	Addr toFreeAddr = (Addr)((char *)block - sizeof(Header));
	Header *toFreeChunk = (Header *)toFreeAddr;   

	// check block is an allocated chunk within the heap
	if (toFreeChunk->status != ALLOC) {
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}

	Header *currChunk = NULL;
    int i = 0;
    // searches for possible free chunk merges
    for (i = 0; i < nFree; i++) {
        // no free chunk surrounding block
        if ((Addr)((char *)freeList[i] - toFreeChunk->size) > toFreeAddr) break; 

        currChunk = (Header *)freeList[i];
        
        // merging block and its previous chunk
        if ((Addr)((char *)freeList[i] + currChunk->size) < (Addr)((char *)heapMem + heapSize) &&  
            (Addr)((char *)freeList[i] + currChunk->size) == toFreeAddr) {
            currChunk->size += toFreeChunk->size;
            toFreeChunk = NULL;
            
            // merging merged chunk (previous and block) with the next chunk
            if (i + 1 != nFree && (Addr)((char *)freeList[i+1] - currChunk->size) == freeList[i]) {
                Header *nextFree = (Header *)freeList[i+1];
                currChunk->size += nextFree->size;
                nFree--;
                nextFree = NULL; 
                // remove pointer to the next chunk from freeList 
                for (int j = i + 1; j < nFree; j++) {
                    freeList[j] = freeList[j+1];
                }
            }          
            return;
               
        // merging block and its next chunk
        } else if ((Addr)((char *)freeList[i] - toFreeChunk->size) >= heapMem &&  
                   (Addr)((char *)freeList[i] - toFreeChunk->size) == toFreeAddr) {   
            toFreeChunk->size += currChunk->size;   
            toFreeChunk->status = FREE;
            currChunk = NULL;
            freeList[i] = (Addr)((char *)toFreeChunk);
            return;
        } 
    }
    
    // no free chunk(s) surrounding block
    // set block's status and set a freeList pointer to it
    toFreeChunk->status = FREE;
	int j = 0;
    for (j = nFree; j > i; j--) {
        freeList[j] = freeList[j-1];
    }
    freeList[j] = toFreeAddr;
    nFree++;
}


// convert pointer to offset in heapMem
int  heapOffset(void *p)
{
   Addr heapTop = (Addr)((char *)heapMem + heapSize);
   if (p == NULL || p < heapMem || p >= heapTop)
      return -1;
   else
      return p - heapMem;
}

// dump contents of heap (for testing/debugging)
void dumpHeap()
{
   Addr    curr;
   Header *chunk;
   Addr    endHeap = (Addr)((char *)heapMem + heapSize);
   int     onRow = 0;
    
   curr = heapMem;
   while (curr < endHeap) {
      char stat;
      chunk = (Header *)curr;
      switch (chunk->status) {
      case FREE:  stat = 'F'; break;
      case ALLOC: stat = 'A'; break;
      default:    fprintf(stderr,"Corrupted heap %08x\n",chunk->status); exit(1); break;
      }
      printf("+%05d (%c,%5d) ", heapOffset(curr), stat, chunk->size);
      onRow++;
      if (onRow%5 == 0) printf("\n");
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}
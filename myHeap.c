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

	// initialise freeList array and set first element to the single free chunk
	size /= MIN_CHUNK; 
	freeList = malloc(size*sizeof(Addr));
	if (freeList == NULL) {
		perror("Out of memory");
		return -1;
	}
	freeList[0] = (Addr)((char *)chunk);
	nFree = 1;
	freeElems = size - 1;

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
	if (nFree == 0 || size < 0 || size > heapSize) return NULL;

	Header *chunk = NULL;
	Header *smallestFreeChunk = NULL;
	int smallestChunkIndex = 0; 
	
	// find smallest free chunk
	for (int i = 0; i < nFree; i++) {
		chunk = (Header *)freeList[i];
		// get first free chunk with size larger than size + headerSize
		if (smallestFreeChunk == NULL  &&  chunk->size > size + sizeof(Header)) {
			smallestChunkIndex = i;
			smallestFreeChunk = (Header *)freeList[i];
		// skip next else if 
		} else if (smallestFreeChunk == NULL) {

		// sets current chunk to be the smallest chunk 	
		} else if (smallestFreeChunk->size > chunk->size  &&  chunk->size > size + sizeof(Header)) {
			smallestFreeChunk = (Header *)freeList[i];	
			smallestChunkIndex = i;
		}
	}	
	
	// no free chunk available
	if (smallestFreeChunk == NULL) return NULL;

	// is it <= or >=     same as above is chunk->size >= ?   ie  what happens when equal
	// free chunk < size + headerSize + MIN_CHUNK, allocate whole chunk 
	if (smallestFreeChunk->size < size + sizeof(Header) + MIN_CHUNK) {
		smallestFreeChunk->status = ALLOC;
		// remove smallest chunk from free list
		for (int i = smallestChunkIndex; i < nFree; i++) {
			freeList[i] = freeList[i+1];
		}
		nFree--;
		freeElems++;
	} else if (smallestFreeChunk->size > size + sizeof(Header) + MIN_CHUNK) {
		// set lower chunk -> alloc
		int oldSize = smallestFreeChunk->size;
		smallestFreeChunk->status = ALLOC;
		smallestFreeChunk->size = size + sizeof(Header);

		// upper chunk -> new free chunk, move pointer from freelist from old address to new   
		Addr lowerAddr = (Addr)((char *)freeList[smallestChunkIndex] + smallestFreeChunk->size); 
		chunk = (Header *)lowerAddr;  
		chunk->status = FREE;

		chunk->size = oldSize - smallestFreeChunk->size;
		freeList[smallestChunkIndex] = (Addr)((char *)freeList[smallestChunkIndex] + smallestFreeChunk->size);   
	}
/*
   Addr    curr;
   Addr    endHeap = (Addr)((char *)heapMem + heapSize);
    
   curr = heapMem;
   while (curr < endHeap) {
      chunk = (Header *)curr;
      printf("\n curr: %p  %d   %d\n", chunk, chunk->status, chunk->size);
      curr = (Addr)((char *)curr + chunk->size);
   }*/ 
    return (char *)smallestFreeChunk + sizeof(Header); 
}




// free a chunk of memory
void myFree(void *block)
{
	// remember void * === Addr
	// no possibility of 4 merge unless myfree/mymalloc/initheap incorrect

	// no more space in freeList   -> does it matter? -> because not enough space -> high chance of merge -> so liekly have free space -> just in case leave?
	// else not using freeElems   -> ?????? probably a problem 
	if (freeElems <= 0) return; 

	// check if block within the heap 
	Addr heapTop = (Addr)((char *)heapMem + heapSize);
	if (block == NULL || block < heapMem || block >= heapTop) {		// >= because if block = -> size = 0
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}

	// check block is an alocated chunk
	Header *toFreeChunk = (Header *)(Addr)((char *)block - sizeof(Header));
	if (toFreeChunk->status != ALLOC) {
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}

	Header *prevFreeChunk = NULL;
	Header *nextFreeChunk = NULL;

	// FOR WHOLE PROGRAM -> IF NFREE >= SIZE OF FREELIST  -> ??? 

	// CHUNKS ALWAYS FREE UNLESS MISTAKE IN CODE B/C LOOPING THROUGH FREELIST
	// PROBLEM : IF EG TEST 2 -> NFREE ALWAYS 1   -> need different loop method IE REDO WHOLE THING
	for (int i = 1; i < nFree; i++) {
	printf("%p %p %p\n", freeList[i], block, freeList[i-1]);
		if (freeList[i] > block && freeList[i-1] < block) {		// convert to (char *) ?  -> can you compare void * ? -> in heapoffset -> compares
			if (i < nFree) nextFreeChunk = (Header *)freeList[i]; 			
			prevFreeChunk = (Header *)freeList[i-1];	// there shoudl always be a previous sicne loop starts at 1 
			
			if (nextFreeChunk == NULL) {	// || nextFree->status != Free    just in case b/c comment in prev chunk merge
				// no merges
				if (freeList[i-1] + prevFreeChunk->size != toFreeChunk) {
					toFreeChunk->status = FREE;
					// want make freelist[i] empty for freechunk, assumes freelist not full ie nFree < freelist size 
					for (int j = nFree; j > i; j--) {
						freeList[j] = freeList[j-1];
					}
					freeList[i] = (Addr)((char *)toFreeChunk);
					nFree++;
					freeElems--;
				// merge with previous chunk
				} else if (freeList[i-1] + prevFreeChunk->size == toFreeChunk) {
					// no need change freelist because havent added toFreechunk to list
					prevFreeChunk->size += toFreeChunk->size;
					//free(toFreeChunk); 	//???    can only free dynamic mem creation ie malloc so can use if we (header *) -> ? -> maybe set status to -1/etc and size = -1/etc  and dont check if null -> check value
					toFreeChunk = NULL;
				}

			} else if (nextFreeChunk != NULL) {
				// merge with next chunk
				if (freeList[i-1] + prevFreeChunk->size != toFreeChunk && freeList[i] + nextFreeChunk->size == toFreeChunk) {
					toFreeChunk->size += nextFreeChunk->size;
					// replace nextFreeChunk with toFreeChunk(merged with next) in freelist
					freeList[i] = (Addr)((char *)toFreeChunk);
					//free(nextFreeChunk); //???
					nextFreeChunk = NULL;
				// merge with both next and previous
				} else if ( freeList[i-1] + prevFreeChunk->size == toFreeChunk && freeList[i] + nextFreeChunk->size == toFreeChunk) {
					prevFreeChunk->size += toFreeChunk->size + nextFreeChunk->size;
					// remember tofreechunk is not in the free list -> so only 2 (next and previous) -> merge previous with current and next so only next removed
					// move freelist elements down one spot
					for (int j = i + 1; j < nFree; j++) {
						freeList[j] = freeList[j+1];
					}
					nFree--;
					freeElems++;
					//free(toFreeChunk);
					//free(nextFreeChunk);
					toFreeChunk = nextFreeChunk = NULL;
				}
			}
		    return;	
		}
	}

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
      //printf("dump : %p  %d   %d\n", chunk, chunk->status, chunk->size);
      switch (chunk->status) {
      case FREE:  stat = 'F'; break;
      case ALLOC: stat = 'A'; break;
      default:    fprintf(stderr,"Corrupted heap %08x\n",chunk->status); exit(1); break;
      }
      printf("+%05d (%c,%5d) ", heapOffset(curr), stat, chunk->size);
      onRow++;
      //if (onRow%5 == 0) printf("\n");
 //     printf("\ndump : %p %p %p\n", curr, curr+8, curr+16);     // why is curr+16  -> +2  ie max + 10 overall
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}

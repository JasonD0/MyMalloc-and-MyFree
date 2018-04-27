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
//    printf("%p %p\n", heapMem, (char *)heapMem + 16);   ...0f  ...10 -> 10 is one after f -> look at hex table 
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
	// no more space in freeList   -> does it matter? -> because not enough space -> high chance of merge -> so liekly have free space -> just in case leave?
	// else not using freeElems   -> ?????? probably a problem 
	if (freeElems <= 0) return; 

	// check if block within the heap 
	Addr heapTop = (Addr)((char *)heapMem + heapSize);
	if (block == NULL || block < heapMem || block >= heapTop) {		// >= because if block = -> size = 0
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}
	
	Addr toFreeAddr = (Addr)((char *)block - sizeof(Header));

	// check block is an alocated chunk
	Header *toFreeChunk = (Header *)toFreeAddr;   /// because data allocated 8 bytes(Header) ahead of start -> no status there
	
	if (toFreeChunk->status != ALLOC) {
		fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}

	Header *currChunk = NULL;
    int i = 0;
    
    for (i = 0; i < nFree; i++) {
        if ((Addr)((char *)freeList[i]) > toFreeAddr) break;
        currChunk = (Header *)freeList[i];
        
        // merging with previous node relative to block
        if ((Addr)((char *)freeList[i]) + currChunk->size < (Addr)((char *)heapMem + heapSize) &&   //prev node last chunk(ensure prev to node)
            (Addr)((char *)freeList[i] + currChunk->size) == toFreeAddr) {
            currChunk->size += toFreeChunk->size;
            toFreeChunk = NULL;
            // merging previous and next node relative to block
            // here if i+1 exists -> next block always exists b/c i exists
            if (i + 1 != nFree && (Addr)((char *)freeList[i+1] - currChunk->size) == freeList[i]) {
                Header *nextFree = (Header *)freeList[i+1];
                currChunk->size += nextFree->size;
                nFree--;
                freeElems++;
                nextFree = NULL;  // note dont think it matters -> bc eg dumpheap -> iterating through by curr size -> ie skips anything between
                // move list 
                for (int j = i + 1; j < nFree; j++) {
                    freeList[j] = freeList[j+1];
                }
            }          
            return;   
        }
        // merging with next node relative to block
        else if ((Addr)((char *)freeList[i]) - toFreeChunk->size > heapMem &&  //next not first chunk in heapMem (ensure always next to a node)
                 (Addr)((char *)freeList[i] - toFreeChunk->size) == toFreeAddr) {   
            toFreeChunk->size += currChunk->size;   
            toFreeChunk->status = FREE;
            currChunk = NULL;
            freeList[i] = (Addr)((char *)toFreeChunk);
            // number of free elements dont change since 2 became 1
            // merging next next shouldnt be possible -> should be already merged 
            return;
        } 
    }
    // get here if no merges 
    toFreeChunk->status = FREE;
/*    if (i <= 0) {
        fprintf(stderr, "Attempt to free unallocated chunk\n");
		exit(1);
	}*/    // wrong because 1st free chunk could be ahead of allocated chunk -> so i =0 -> break;
	// i -> index where 1st bigger than block -> ie block is before i -> so want block at i  
	    // ie 3rd place passes 2nd place -> becomes 2nd place
	// no merges, adding new free chunk to freeList
	int j = 0;
    for (j = nFree; j > i; j--) {
        freeList[j] = freeList[j-1];
    }
    freeList[j] = toFreeAddr;
    nFree++;
    freeElems--;

/*
	// PROBLEM : IF EG TEST 2 -> NFREE ALWAYS 1   -> need different loop method IE REDO WHOLE THING
	    // free first -> then loop freelist -> check if their addr + size == freed start adress   
	        // NOTE: REMEMBER TO MINUS SIZEOF HEADER WHEN NECESSARY
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
					toFreeChunk = NULL;
				}

			} else if (nextFreeChunk != NULL) {
				// merge with next chunk
				if (freeList[i-1] + prevFreeChunk->size != toFreeChunk && freeList[i] + nextFreeChunk->size == toFreeChunk) {
					toFreeChunk->size += nextFreeChunk->size;
					// replace nextFreeChunk with toFreeChunk(merged with next) in freelist
					freeList[i] = (Addr)((char *)toFreeChunk);
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
					toFreeChunk = nextFreeChunk = NULL;
				}
			}
		    return;	
		}
	}*/

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
      if (onRow%5 == 0) printf("\n");
 //     printf("\ndump : %p %p %p\n", curr, curr+8, curr+16);     // why is curr+16  -> +2  ie max + 10 overall
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}

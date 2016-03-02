#include <stdint.h>

#define MAX_FREE_MEMORY 131072 //1024 bytes/KB *128. 128KB*/
#define FREE 0
#define ALLOCATED 1
#define FIRSTFIT 2
#define BESTFIT 3
#define METADATA 10 //each node has 32 bytes of meta data
#define META_TAG 1
#define META_SIZE 4
#define MIN_SPLIT 10 //When allocating memory, if the available free block is larger than required, must have at least this minimum number of bytes in the resulting free block from the split
#define EXTRA_HEAP_TOP 5120 //When allocated memory and the program break has to be adjusted, increase by the required amount (for the allcoation) + this EXTRA_HEAP_TOP to reduced sbrk calls


/*Returns void pointer to the memory, if memory could not be allocated, returns NULL and 
sets my_malloc_error (global error string)*/
void *my_malloc(int size); 
/*Deallocates block of memory pointed by the ptr argument*/
void my_free(void *ptr);
/*Specifies the memory allocation policy, either first fit or best fit*/
void my_mallopt(int policy);
/*Prints statistics about the memory allocation performed so far by the library
including total number of bytes allocated, total free speac, largest contiguous free space and others.*/
void my_mallinfo(void);


//Helper functions

int getSizeNode(void* address);
char getAllocatedTag(void* address);
void updateNode(void* address, int size, char allocated);

void printNode(void* address);
char nextNodeFree(void* address);
char previousNodeFree(void* address);

void* nextNode(void* address);
void* previousNode(void* address);
void* firstFit(int size);
void* bestFit(int size);
void mergeUp(void* address);
void mergeDown(void* address);







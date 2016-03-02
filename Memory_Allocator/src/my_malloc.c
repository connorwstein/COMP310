#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "my_malloc.h"


int policy=FIRSTFIT;
char* heapBottom; //points to the first byte at the bottom of the heap
char* heapTop;
int uninitialized=1;
int totalFree=0;
int totalAllocated=0;
int totalMeta=0;
char malloc_error[]="Error, sbrk returned -1";
extern char *my_malloc_error=malloc_error;




int getHeapSize(void){
	return ((int)heapTop-(int)heapBottom);
}

int getTotalBytes(void){
	return totalAllocated+totalMeta+totalFree;
}

int getSizeNode(void* address){

	return *((int*)(address+META_TAG));
}
char getAllocatedTag(void* address){

	return *((char*)(address));
}
void updateNode(void* address, int size, char allocated){
	void *currentPointer=address;
	*((char*)currentPointer)=allocated;
	currentPointer+=META_TAG;
	*((int*)currentPointer)=size;
	currentPointer+=(size+META_SIZE);
	*((int*)currentPointer)=size;
	currentPointer+=META_SIZE;
	*((char*)currentPointer)=allocated;
}

void printNode(void* address){
	void *currentPointer=address;
	int size=getSizeNode(address);
	char tag1=*((char*)currentPointer);
	currentPointer+=META_TAG;
	int size1=*((int*)currentPointer);
	currentPointer+=(size+META_SIZE);
	int size2=*((int*)currentPointer);
	currentPointer+=META_SIZE;
	char tag2=*((char*)currentPointer);
	printf("Node %d: [%d,%d,%d,%d] Heap: [%d,%d] \n",(int)address,tag1,size1,size2,tag2,(int)heapBottom,(int)heapTop);
}

char nextNodeAllocated(void* address){
	void *nextNode=address;
	nextNode=address+META_TAG+META_SIZE+getSizeNode(address)+META_SIZE+META_TAG;
	if(nextNode==heapTop){
		//is no next node, so not free for merge
		return -1;
	}
	return *((char*)(nextNode));
}
char previousNodeAllocated(void* address){
	if(address==heapBottom){
		//no previous node
		return -1;
	}
	return *((char*)(address-META_TAG));
}
void* nextNode(void* address){
	void *nextNode=address;
	nextNode=address+META_TAG+META_SIZE+getSizeNode(address)+META_SIZE+META_TAG;
	if(nextNode==heapTop ||address==heapTop){
		return NULL; //no next node
	}
	return nextNode;
}
void* previousNode(void* address){
	void *previousNode=address;
	
	if(address==heapBottom){
		return NULL;
	}
	int size=*((int*)(address-META_TAG-META_SIZE));
	previousNode=address-META_TAG-META_SIZE-size-META_SIZE-META_TAG;
	return previousNode;
}
void mergeUpDown(void* address){
	int size1=getSizeNode(previousNode(address));
	int size2=getSizeNode(address);
	int size3=getSizeNode(nextNode(address));
	updateNode(previousNode(address),size1+size2+size3+METADATA,getAllocatedTag(nextNode(address)));
	totalFree+=(size2+2*METADATA);
	totalMeta-=2*METADATA;
	totalAllocated-=size2;
}
void mergeUp(void* address){
	//Assuming will be called on valid merges i.e. when there is a block higher in the heap to be merged
	//When merging we really just need to update the length in both blocks, can forget about the stuff in the middle it will be junk
	int size1=getSizeNode(address);
	int size2=getSizeNode(nextNode(address));
	updateNode(address,size1+size2+METADATA,getAllocatedTag(nextNode(address)));
}
void mergeDown(void* address){
	int size1=getSizeNode(previousNode(address));
	int size2=getSizeNode(address);
	updateNode(previousNode(address),size1+size2+METADATA,getAllocatedTag(previousNode(address)));
	totalFree+=(size2+METADATA); //one meta become data in a merge
	totalMeta-=METADATA;
	totalAllocated-=size2;
}
void* firstFit(int size){
	void* result=NULL;
	//loop from bottom of the heap looking for a allocated tag equal to 0 i.e. free
	char* pointer=heapBottom; //initialize the looping byte pointer to the bottom of the heap
	while(pointer<=heapTop){
		printf("Heap: %d Total: %d\n",getHeapSize(),getTotalBytes());
		if(getAllocatedTag(pointer)==0){
			//found a free block
			printf("Found free block %d\n",(int)pointer);
			if(getSizeNode(pointer)+METADATA>=size+METADATA*2+MIN_SPLIT){
				//Enough room to split
				int sizeDelta=getSizeNode(pointer)-size;
		
				
				updateNode(pointer,size,1);
				void *newFree=nextNode(pointer);
				printf("New free %d\n",(int)(int*)newFree);
				printf("Allocated part free block\n");
				printNode(pointer);
				updateNode(newFree,sizeDelta-METADATA,0);
				printf("Free part of free block\n");
				printNode(newFree);
				//check for merge
				// if((nextNodeAllocated(pointer)!=-1)&&!nextNodeAllocated(pointer)){
				// 	printf("Merging after re-allocating\n");
				// 	mergeUp(pointer);
				// 	printNode(pointer);

				// }
				
				totalMeta+=METADATA;
				totalFree-=(size+METADATA);
				totalAllocated+=size;
				return pointer;
				
			
			}
			else if(getSizeNode(pointer)>=size){
				//Not enough room to split, but big enough to overwrite, give extra space to malloc caller
				updateNode(pointer,getSizeNode(pointer),0); //just make the same whole block unallocated
				totalFree-=getSizeNode(pointer);
				totalAllocated+=getSizeNode(pointer);
				return pointer;
			}
		}
		pointer=nextNode(pointer);
		if(pointer==NULL){
			break;
		}
	}
	void *oldHeapTop=sbrk(size+2*METADATA+EXTRA_HEAP_TOP);
	if(oldHeapTop==-1){
		printf("%s\n",my_malloc_error);
		return -1;
	}
	heapTop+=(size+2*METADATA+EXTRA_HEAP_TOP);
	printf("No free blocks large enough available. Have to increase the heap from %d to %d\n",(int)((int*)oldHeapTop),(int)heapTop);
	updateNode(oldHeapTop,size,1);
	printNode(oldHeapTop);
	updateNode(oldHeapTop+size+METADATA,EXTRA_HEAP_TOP,0);
	printNode(oldHeapTop+size+METADATA);
	totalMeta+=2*METADATA;
	totalAllocated+=size;
	totalFree+=EXTRA_HEAP_TOP;
	return oldHeapTop;
}
void* bestFit(int size){

}
void *my_malloc(int size){
	void* result=NULL; //Initialize the result to null
	//NOTE RESULT IS POINTER TO BEGINNING OF WHOLE NODE
	//WILL NEED TO CHANGE TO RETURN POINTER TO DATA
	if(uninitialized){
		printf("First Malloc: size %d\n",size);
		 //First time malloc is called, save the address of the bottom of the heap
		heapBottom=(char*)sbrk(size+2*METADATA+EXTRA_HEAP_TOP); //create free space on the heap
		if(heapBottom==-1){
			printf("%s\n",my_malloc_error);
			return -1;
		}
		heapTop=heapBottom+size+2*METADATA+EXTRA_HEAP_TOP;
		uninitialized=0;
		updateNode(heapBottom,size,1);
		printNode(heapBottom);
		updateNode(heapBottom+size+METADATA,EXTRA_HEAP_TOP,0); //add the extra free space from extra heap top
		printNode(heapBottom+size+METADATA);
		totalMeta+=2*METADATA;
		totalAllocated+=size;
		totalFree+=EXTRA_HEAP_TOP;
		return (void*)heapBottom;
	}
	if(policy==FIRSTFIT){
		result=firstFit(size);
	}
	else{
		result=bestFit(size);
	}

	return result;
}
/*Deallocates block of memory pointed by the ptr argument*/
void my_free(void *ptr){
/*Should reduce the program break if the top free block is larger than 128KB*/
	printf("Here");
	if(ptr==NULL){
		return;
	}
	printf("Here");
	printf("Next node free: %d, previous node free: %d\n",nextNodeAllocated(ptr),previousNodeAllocated(ptr));
	if((nextNodeAllocated(ptr)!=-1)&&(previousNodeAllocated(ptr)!=-1)&&!nextNodeAllocated(ptr)&&!previousNodeAllocated(ptr)){
		//merge both sides
		printf("Freeing node, merging both sides: %d\n",(int)ptr);
		mergeUpDown(ptr);
	}
	else if((nextNodeAllocated(ptr)!=-1)&&!nextNodeAllocated(ptr)){
		//merge up
		printf("Freeing node, merging up: %d\n",(int)ptr);
		mergeUp(ptr);
	}
	else if((previousNodeAllocated(ptr)!=-1)&&!previousNodeAllocated(ptr)){
		//merge down
		printf("Freeing node, merging down: %d\n",(int)ptr);
		mergeDown(ptr); //merging down removes a META_DATA and increases the free

	}
	else{
		printf("Freeing node: %d\n",(int)ptr);
		updateNode(ptr,getSizeNode(ptr),0);
		totalFree+=getSizeNode(ptr); 
		totalAllocated-=getSizeNode(ptr);
	}
	//Check top free block
}
/*Specifies the memory allocation policy, either first fit or best fit*/
void my_mallopt(int policy){
	policy=policy;
}
/*Prints statistics about the memory allocation performed so far by the library
including total number of bytes allocated, total free speac, largest contiguous free space and others.*/
void my_mallinfo(void){
	printf("Allocated: %d Free: %d Metadata %d\n",totalAllocated,totalFree,totalMeta);
}


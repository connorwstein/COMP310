#include <stdio.h>
#include "my_malloc.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void printNodes(int* mallocResults,int numMallocs){
	int k;
	for(k=0;k<numMallocs;k++){
		printNode(mallocResults[k]);
	}
}

void generateMallocSizes(int* mallocSizeArray, int numMallocs){
	int k;
	for(k=0;k<numMallocs;k++){
		mallocSizeArray[k]=rand()%10000; //keep mallocs on order of KB
	}
}

void applyMallocs(int* mallocResultsArray, int* mallocSizeArray, int numMallocs){
	int k;
	for(k=0;k<numMallocs;k++){
		mallocResultsArray[k]=my_malloc(mallocSizeArray[k]);
	}
}
void printIntArray(char* name,int* array, int size){
	int k;
	printf("%s [",name);
	for(k=0;k<size;k++){
		if(k!=size-1){
			printf("%d,",array[k]);
		}
		else{
			printf("%d",array[k]);
		}
	}
	printf("]\n");
}
int testHeapSize(void){
	if(getHeapSize()!=getTotalBytes()){
		return -1;
	}
	else{
		return 0;
	}
}
int main(void){

	srand(time(NULL)); //Seed the random number generator to get different number for every run
	/******************************************************************/
	/* First test, do a bunch of mallocs consecutively without freeing*/
	/******************************************************************/
	int numMallocs=5;
	int mallocSizes[numMallocs],mallocResults[numMallocs];
	generateMallocSizes(mallocSizes,numMallocs);
	printIntArray("Malloc Sizes",mallocSizes, numMallocs);
	applyMallocs(mallocResults,mallocSizes,numMallocs);
	printIntArray("Malloc Results",mallocResults, numMallocs);
	if(testHeapSize()<0){
		printf("ERROR: Heap size does not match total bytes in heap. Heap %d, total %d\n",getHeapSize(),getTotalBytes());
	}
	else{
		printf("Heap size is correct.\n");
	}
	//void* firstNode=(void*)mallocResults[0];
	//printNodes(mallocResults,numMallocs);
	/******************************************************************/
	/* Second test, free those mallocs in random order 				  */
	/******************************************************************/
	//Shuffle the array of results with Knuth shuffle
	// int k;
	// int j;
	// for(k=numMallocs-1;k>0;k--){
	// 	j=rand()%k;
	// 	int temp=mallocResults[j];
	// 	mallocResults[j]=mallocResults[k];
	// 	mallocResults[k]=temp;
	// }
	// printIntArray("Malloc Results After Shuffle",mallocResults, numMallocs);
	// //Now free
	
	my_free((void*)mallocResults[0]);
	// // int i;
	// // for(i=0;i<numMallocs;i++){
	// 	my_free(mallocResults[i]);
	// }
	if(testHeapSize()<0){
		printf("ERROR: Heap size does not match total bytes in heap\n");
	}
	else{
		printf("Heap size is correct.\n");
	}
	//printNode(firstNode);

	return 0;
}
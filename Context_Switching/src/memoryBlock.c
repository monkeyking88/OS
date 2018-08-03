#include "memoryBlock.h"
#include <stdlib.h>


MEM_BLK* initMemory(){
	int i = 0;
	MEM_BLK* first;
	MEM_BLK* temp;
	MEM_BLK* previous = NULL;
	for (i = 0; i < MEM_BLK_NUM; i++){
		temp = (MEM_BLK*)malloc(sizeof(MEM_BLK));
		temp->isUsed = -1;
		temp->pid = 0;
		temp->next = NULL;
		temp->memory = (void*)malloc(MEM_SIZE);
		
		if (previous != NULL){
			previous->next = temp;
		}
		previous = temp;
		if (i == 0){
			first = temp;
		}
	}
	
	return first;
}


int isFree(MEM_BLK* blk){
	if (blk->isUsed == 0){
		return 0;
	}
	else{
		return 1;
	}
}

MEM_BLK* findAvailableBlock(MEM_BLK* first){
	MEM_BLK* temp = first;
	while (temp != NULL && isFree(temp) == 0){
		temp = temp->next;
	}
	return temp;
}

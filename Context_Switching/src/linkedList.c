
#include "linkedList.h"
#include <stdlib.h>

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */


READY_Q*	initREADYQ(){
	int i = 0;

	READY_Q* readyQ;
	readyQ = (READY_Q*)malloc(sizeof(READY_Q) * 4);
	
	for (i = 0; i < 4; i++){
		readyQ[i].first = NULL;
		readyQ[i].last = NULL;
	}
	
	return readyQ;
}

READY_Q* pushToQ(PCB* pcb, READY_Q* readyQ){
	int priority = pcb->m_priority;
	READY_Q_BLK* old_last = readyQ[priority].last;
	
	//allocate new memory for the new block
	READY_Q_BLK* newBlock = (READY_Q_BLK*)malloc(sizeof(READY_Q_BLK));
	newBlock->next = NULL;
	newBlock->pcb = pcb;
	
	//point the last pointer to the new block

	if (old_last != NULL){
		old_last->next = newBlock;
	}
	readyQ[priority].last = newBlock;
	
	//if the list was empty, also point the first to the new block
	if (readyQ[priority].first == NULL){
		readyQ[priority].first = newBlock;
	}
	
	return readyQ;
}

READY_Q_BLK* dequeueFromQ(int priority, READY_Q* readyQ){
	READY_Q_BLK* firstNode = readyQ[priority].first;
	if (readyQ[priority].first != NULL){
		readyQ[priority].first = readyQ[priority].first->next;
	}
	return firstNode;
}

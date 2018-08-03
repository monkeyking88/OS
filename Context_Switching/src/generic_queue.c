#include "generic_queue.h"

void pushToQueue(PCB *pcb, PCB** queue){
	int priority;
	
	if (pcb == NULL){
		return;
	}
	priority = pcb->m_priority;
	pcb->mp_next=NULL;
	if (queue[priority] == NULL){
		queue[priority] = pcb;
	}
	else{
		PCB* temp_blk = queue[priority];
		while(temp_blk->mp_next != NULL){
			temp_blk = temp_blk->mp_next;
		}
		temp_blk->mp_next = pcb;
	}
}

void pushToQueueAtFront(PCB *pcb, PCB** queue){
	int priority;
	
	if (pcb == NULL){
		return;
	}
	priority = pcb->m_priority;
	pcb->mp_next=NULL;
	if (queue[priority] == NULL){
		queue[priority] = pcb;
	}
	else{
		pcb->mp_next = queue[priority];
		queue[priority] = pcb;
	}
}

void eraseFromQueue(PCB *pcb, PCB** queue){
	int priority;
	PCB* firstNode;
	PCB* thatNode;
	
	if (pcb == NULL){
		return;
	}
	//find out if the pcb was in queue, remove it from its previous position
	priority = pcb->m_priority;
	firstNode = queue[priority];
	if (firstNode == NULL){
		return;
	}
	else if (firstNode->m_pid == pcb->m_pid){
		//first in queue, ignore in queue
		thatNode = firstNode;
		queue[priority] = firstNode->mp_next;
		thatNode->mp_next = NULL;
	}
	else{
		//if the pcb was in the queue but not at the front
		while (firstNode->mp_next != NULL && firstNode->mp_next->m_pid != pcb->m_pid){
			firstNode = firstNode->mp_next;
		}
		if (firstNode->mp_next != NULL){
			thatNode = firstNode->mp_next;
			firstNode->mp_next = firstNode->mp_next->mp_next;
			thatNode->mp_next = NULL;
		}
	}
}


PCB* popFromQueue(PCB** queue){
	int i = 0;
	PCB* nextProcess = NULL;
	while (i < 5 && nextProcess == NULL){
		nextProcess = popFromQueueWithPriority(i, queue);
		i++;
	}
	return nextProcess;
}


PCB* popFromQueueWithPriority(int priority, PCB** queue){
	PCB* firstNode = queue[priority];
	if (queue[priority] != NULL){
		queue[priority] = queue[priority]->mp_next;
		firstNode->mp_next = NULL;
	}
	return firstNode;
}

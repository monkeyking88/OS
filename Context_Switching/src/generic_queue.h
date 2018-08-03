#include "k_rtx.h"

#ifndef GENERIC_QUEUE_H_
#define GENERIC_QUEUE_H_



void pushToQueue(PCB *pcb, PCB** queue);
void pushToQueueAtFront(PCB *pcb, PCB** queue);
void eraseFromQueue(PCB *pcb, PCB** queue);
PCB* popFromQueue(PCB** queue);
PCB* popFromQueueWithPriority(int priority, PCB** queue);

#endif

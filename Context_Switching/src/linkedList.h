#include "k_rtx.h"


#define ARR_SIZE 4;

typedef struct ready_q_blk{
	struct ready_q_blk *next;
	PCB *pcb;
} READY_Q_BLK;


typedef struct ready_q { 
	READY_Q_BLK *first;
	READY_Q_BLK *last;
} READY_Q;

#include "k_rtx.h"
#include "msg.h"

#ifndef MEMORYBLOCK_H_
#define MEMORYBLOCK_H_

#define MEM_SIZE 128
#define MEM_BLK_NUM 2


typedef struct mem_blk { 
	struct mem_blk *next;
	int isUsed;
	
	int sendTime;
	int send_id;
	int dest_id;
	struct msgbuf *nextMsg;
	
} MEM_BLK;


#endif

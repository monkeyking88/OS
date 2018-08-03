/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "memoryBlock.h"
#include "k_process.h"
#include "uart_polling.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */
U32 *gp_heap;
extern PCB *gp_current_process;
/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack (drowing downwards)
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<--- gp_heap (growing upwards)
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	int totalLength;
  U32* current = gp_heap;
	MEM_BLK* next_mem_blk;
	MEM_BLK* curr_mem_blk;
	
	
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TOTAL_PROCS) * sizeof(PCB *);
		
	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
	#ifdef DEBUG_0  
	//printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	#endif
	
	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		//printf("gp_pcbs = 0x%x \n", gp_pcbs[i]);
	}
	
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	/* allocate memory for heap, not implemented yet*/
  gp_heap = (U32*) p_end;
	
	if ((U32)gp_heap & 0x04) {
		++gp_heap; 
	}
	
	
	totalLength = (sizeof(MEM_BLK)+(MEM_SIZE/sizeof(int)))*MEM_BLK_NUM;
	current = gp_heap;
	for (i = 0; i < totalLength; i++){
		*current = 0;
		current++;
	}
	
	next_mem_blk = (MEM_BLK*)gp_heap;
	for (i = 0; i < MEM_BLK_NUM; i++){
		curr_mem_blk = next_mem_blk;
		curr_mem_blk->isUsed = 0;
		curr_mem_blk->send_id = 0;
		curr_mem_blk->dest_id = 0;
		curr_mem_blk->next = NULL;

		next_mem_blk += sizeof(MEM_BLK);
		next_mem_blk += (MEM_SIZE/sizeof(int));
		
		if (i != MEM_BLK_NUM - 1){
			curr_mem_blk->next = next_mem_blk;
		}
		else{
			curr_mem_blk->next = NULL;
		}
	}	
	
	//uart0_put_string("\r");
	//printf("MEM_BLK_AVL: %d\n", get_num_available());	
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void *k_request_memory_block(void) {
	MEM_BLK* availableBlock = getAvailableMemBlk();
	//return (void*)(gp_heap+sizeof(MEM_BLK));
	
	#ifdef DEBUG_0 
	//printf("k_request_memory_block: entering...\n");
	#endif /* ! DEBUG_0 */
	
	if (availableBlock == NULL){
		//if no memory, push the current process to blocked queue once
		pushToBlockedQ(gp_current_process);
	}
	while (availableBlock == NULL){
		gp_current_process->m_state = BLOCKED;
		k_release_processor();
		availableBlock = getAvailableMemBlk();
	}
		
	availableBlock->isUsed = 1;
	availableBlock->send_id = 0;
	availableBlock->dest_id = 0;
	availableBlock->next = NULL;
	
	availableBlock = availableBlock+sizeof(MEM_BLK);
	
	//uart0_put_string("\r");
	//printf("MEM_BLK_AVL: %d\n", get_num_available());	
	return (void*)(availableBlock);
}

int k_release_memory_block(void *p_mem_blk) {
	U32* blk_ptr;
	int size = MEM_SIZE/sizeof(int);
	int i;
	PCB* pop_blocked_ptr = popFromBlockedQ();
	MEM_BLK* mem_blk = (MEM_BLK*)((MEM_BLK*)p_mem_blk - sizeof(MEM_BLK));
	
	
	#ifdef DEBUG_0 
	//printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
	#endif /* ! DEBUG_0 */
	if (is_p_block_valid(mem_blk) == 0){
		return RTX_ERR;
	}
	//set the block to be free
	returnMemBlk(mem_blk);

	blk_ptr = (U32*)p_mem_blk;
	for (i = 0; i < size; i++){
		*blk_ptr = 0;
		blk_ptr++;
	}
	
	//if there are processes being blocked, release the last one in the blocked queue
	if (pop_blocked_ptr != NULL){
		//handle_process_ready adds a pcb just removed from the blocked queue to the ready queue
		//release the processor
		handle_process_ready_from_blocked(pop_blocked_ptr);
	}
	return RTX_OK;
}

MEM_BLK* getAvailableMemBlk(){
	int counter = 0;
	int found = 0;
	MEM_BLK* mem_blk_ptr = (MEM_BLK*)gp_heap;
	while(counter < MEM_BLK_NUM && found == 0){
		if (mem_blk_ptr -> isUsed == 1){
			mem_blk_ptr = (mem_blk_ptr+sizeof(MEM_BLK)+(MEM_SIZE/4));	
		}
		else{
			found = 1;
		}
		counter++;
	}
	if (found == 1){
			return mem_blk_ptr;
	}
	else{
			return NULL;
	}
}

void returnMemBlk(MEM_BLK* mem_blk){
	mem_blk->isUsed = 0;
	mem_blk->send_id = 0;
	mem_blk->dest_id = 0;
	mem_blk->next = NULL;
	
}

int is_p_block_valid(MEM_BLK* p_mem_blk){
	int i = 0;
	MEM_BLK* valid_start = (MEM_BLK*)gp_heap;
	
	#ifdef DEBUG_0 
	//printf("is_p_block_valid @ 0x%x\n", p_mem_blk);
	#endif /* ! DEBUG_0 */
	if (p_mem_blk == NULL){
		return 0;
	}
	
	for (i = 0; i < MEM_BLK_NUM; i++){
		if (valid_start == p_mem_blk){
			return 1;
		}
		valid_start = valid_start+(MEM_SIZE/4)+sizeof(MEM_BLK);
	}
	return 0;
}

int get_num_available(){
	int counter = 0;
	int num_available = 0;
	MEM_BLK* mem_blk_ptr = (MEM_BLK*)gp_heap;
	while(counter < MEM_BLK_NUM){
		if (mem_blk_ptr -> isUsed != 1){
			num_available = num_available + 1;
		}
		mem_blk_ptr = (mem_blk_ptr+sizeof(MEM_BLK)+(MEM_SIZE/4));
		counter++;
	}
	return num_available;
}


/**
 * @file:   k_memory.h
 * @brief:  kernel memory managment header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_MEM_H_
#define K_MEM_H_

#include "k_rtx.h"
#include "memoryBlock.h"
#include "k_process.h"

/* ----- Definitions ----- */
#define RAM_END_ADDR 0x10008000

/* ----- Variables ----- */
/* This symbol is defined in the scatter file (see RVCT Linker User Guide) */  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit; 
extern PCB **gp_pcbs;
extern PROC_INIT g_proc_table[NUM_TOTAL_PROCS];
extern int k_release_processor(void);

/* ----- Functions ------ */
void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);
MEM_BLK* getAvailableMemBlk(void);
void returnMemBlk(MEM_BLK* p_mem_blk);
int is_p_block_valid(MEM_BLK* p_mem_blk);
int get_num_available(void);


#endif /* ! K_MEM_H_ */

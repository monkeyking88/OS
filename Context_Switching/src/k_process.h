/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include "k_rtx.h"
#include "generic_queue.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */

/* ----- Functions ----- */
PCB ** getgp_pcbs(void);
PCB * getcurrentProcess(void);

void pushToReadyQ(PCB *pcb);
void pushToReadyQByChangeOfPriority(PCB *pcb);
void eraseFromReadyQByChangeOfPriority(PCB *pcb);
PCB* popFromReadyQ(void);

void pushToBlockedQ(PCB* pcb);
void pushToBlockedQByChangeOfPriority(PCB *pcb);
void eraseFromBlockedQByChangeOfPriority(PCB *pcb);
PCB* popFromBlockedQ(void);


void handle_process_ready_from_blocked(PCB* pcb);
int k_set_process_priority(int process_id, int priority);
int k_get_process_priority(int process_id);
void nullProc(void);
void timer_i(void);

//the following are all place holders that will either be implemented shortly after or in P3
void setPriorityProcess(void);
void wallClockDisplay(void);
void kcd(void);
void crt(void);
void uart_i(void);


void process_init(void);               /* initialize all procs in the system */
void initStressProcesses(void);				 /* initialize A, B, C processes */
void initSystemProcesses(void);				 /* initialize all the system process */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);         /* kernel release_process function */
void handleTimerInterrupt(void);			 /* kernel handles timer interrupt */
void null_process(void);							 /* null process */
extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */
void ato_on(void);
void ato_off(void);

#endif /* ! K_PROCESS_H_ */

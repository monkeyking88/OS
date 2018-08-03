/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"
#include "k_memory.h"
#include "stress.h"
#include "msg.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */
PCB *readyQueue[5] = {NULL,NULL,NULL,NULL, NULL};/* ReadyQueue Initialization*/
PCB *blockedQueue[5] = {NULL,NULL,NULL,NULL, NULL}; /*BlockedQueue Initialization*/

/* process initialization table */
PROC_INIT g_proc_table[NUM_TOTAL_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

void pushToReadyQ(PCB *pcb){
	pushToQueue(pcb, readyQueue);
}

void pushToReadyQByChangeOfPriority(PCB *pcb){
	pushToQueue(pcb, readyQueue);
}

void eraseFromReadyQByChangeOfPriority(PCB *pcb){
	eraseFromQueue(pcb, readyQueue);
}

PCB* popFromReadyQ(){
	return popFromQueue(readyQueue);
}


void pushToBlockedQ(PCB *pcb){
	pushToQueue(pcb, blockedQueue);
}

void pushToBlockedQByChangeOfPriority(PCB *pcb){
	pushToQueue(pcb, blockedQueue);
}

void eraseFromBlockedQByChangeOfPriority(PCB *pcb){
	eraseFromQueue(pcb, blockedQueue);
}

PCB* popFromBlockedQ(){
	return popFromQueue(blockedQueue);
}



/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init() {
	int i = 0;
	U32 *sp;  
	
	/* fill out the initialization table */
	set_test_procs();
		
	//define the null process
	g_proc_table[0].m_pid=(U32)(PID_NULL);
	//NULL process has priority 4
	g_proc_table[0].m_priority = 4;
	g_proc_table[0].m_stack_size=0x100;
	g_proc_table[0].mpf_start_pc = &nullProc;
	
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i+1].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i+1].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i+1].mpf_start_pc = g_test_procs[i].mpf_start_pc;	
		g_proc_table[i+1].m_priority = g_test_procs[i].m_priority;
	}
	
	initStressProcesses();
	initSystemProcesses();
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;


		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		
		(gp_pcbs[i])->mp_sp = sp;		
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;	

		//TODO
		if(i <= 6){
			pushToReadyQ(gp_pcbs[i]);
		}
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 */

PCB *scheduler(void){
	PCB* nextProc = popFromReadyQ();

	if (nextProc == NULL){
		nextProc = gp_current_process;
	}
	return nextProc;
}


/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			if (p_pcb_old != NULL){
				p_pcb_old->mp_sp = (U32 *) __get_MSP();
				if (p_pcb_old->m_state != BLOCKED && p_pcb_old->m_state != BLOCKED != BLOCKED_ON_RECEIVE){
					p_pcb_old->m_state = RDY;

				}
			}
		}

		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */
	if (gp_current_process != p_pcb_old) {
		if (state == RDY){
			if (p_pcb_old != NULL){
				p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			
				if (p_pcb_old->m_state != BLOCKED && p_pcb_old->m_state != BLOCKED_ON_RECEIVE){
						p_pcb_old->m_state = RDY;
				}
			}
			
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		}		
		else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = gp_current_process;
	if (p_pcb_old != NULL && p_pcb_old->m_state == RUN){
		pushToReadyQ(p_pcb_old);
	}
	
	gp_current_process = scheduler();
	
	if (gp_current_process == NULL){
			return RTX_ERR;		
	}
	
	process_switch(p_pcb_old);
	return RTX_OK;
}


void handle_process_ready_from_blocked(PCB* pcb){
	pcb->m_state = RDY;
	pcb->mp_next = NULL;
	pushToReadyQ(pcb);
	k_release_processor();
}


int k_set_process_priority(int process_id, int priority){
	if (process_id < PID_P1 || process_id > PID_P6 || priority < HIGH || priority > LOW){
		return RTX_ERR;
	}
	
	//if it is ready, change it in ready queue
	if(gp_pcbs[process_id]->m_state == RDY || gp_pcbs[process_id]->m_state == NEW){
		eraseFromReadyQByChangeOfPriority(gp_pcbs[process_id]);
		(gp_pcbs[process_id])->m_priority = priority;
		pushToReadyQByChangeOfPriority(gp_pcbs[process_id]);
	}
	//if it is blocked, change it in blocked queue
	else if (gp_pcbs[process_id]->m_state == BLOCKED){
		eraseFromBlockedQByChangeOfPriority(gp_pcbs[process_id]);
		(gp_pcbs[process_id])->m_priority = priority;
		pushToBlockedQByChangeOfPriority(gp_pcbs[process_id]);
	}
	else if (gp_pcbs[process_id]->m_state == RUN){
		(gp_pcbs[process_id])->m_priority = priority;
	}
	
	k_release_processor();
	return RTX_OK;
}

int k_get_process_priority(int process_id){
	if (process_id < PID_P1 || process_id > PID_P6){
		return RTX_ERR;
	}
	return (gp_pcbs[process_id])->m_priority;
}

extern PCB ** getgp_pcbs(void){
	return gp_pcbs;
}
extern PCB * getcurrentProcess(void){
	return gp_current_process;
}



void handleTimerInterrupt(void){
	PCB *p_pcb_old = gp_current_process;
	if (p_pcb_old != NULL && p_pcb_old->m_state == RUN){
		pushToReadyQ(p_pcb_old);
	}
	gp_current_process = gp_pcbs[PID_TIMER_IPROC];
	process_switch(p_pcb_old);
	
	//
	gp_current_process->m_state = RDY;
	
	gp_current_process = scheduler();
	/*
	if (state == RDY){
		if (p_pcb_old != NULL){
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
		
			if (p_pcb_old->m_state != BLOCKED && p_pcb_old->m_state != BLOCKED != BLOCKED_ON_RECEIVE){
					p_pcb_old->m_state = RDY;
			}
		}
		
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
	}
	*/
}

void nullProc(){
	while(1){
		#ifdef DEBUG_0
		uart0_put_string("\r");
		printf("G016_test: proc NULL running\n");
		#endif 
		k_release_processor();
	}
}

void initStressProcesses(){
	g_proc_table[7].m_pid=(U32)(PID_A);
	g_proc_table[7].m_priority = LOWEST;
	g_proc_table[7].m_stack_size=0x100;
	g_proc_table[7].mpf_start_pc = &procA;
	
	
	g_proc_table[8].m_pid=(U32)(PID_B);
	g_proc_table[8].m_priority = LOWEST;
	g_proc_table[8].m_stack_size=0x100;
	g_proc_table[8].mpf_start_pc = &procB;
	
	g_proc_table[9].m_pid=(U32)(PID_C);
	g_proc_table[9].m_priority = LOWEST;
	g_proc_table[9].m_stack_size=0x100;
	g_proc_table[9].mpf_start_pc = &procC;
}

void initSystemProcesses(){
	g_proc_table[10].m_pid=(U32)(PID_SET_PRIO);
	g_proc_table[10].m_priority = HIGH;
	g_proc_table[10].m_stack_size=0x100;
	g_proc_table[10].mpf_start_pc = &setPriorityProcess;
	
	g_proc_table[11].m_pid=(U32)(PID_CLOCK);
	g_proc_table[11].m_priority = HIGH;
	g_proc_table[11].m_stack_size=0x100;
	g_proc_table[11].mpf_start_pc = &wallClockDisplay;
	
	g_proc_table[12].m_pid=(U32)(PID_KCD);
	g_proc_table[12].m_priority = HIGH;
	g_proc_table[12].m_stack_size=0x100;
	g_proc_table[12].mpf_start_pc = &kcd;
	
	g_proc_table[13].m_pid=(U32)(PID_CRT);
	g_proc_table[13].m_priority = HIGH;
	g_proc_table[13].m_stack_size=0x100;
	g_proc_table[13].mpf_start_pc = &crt;
	
	g_proc_table[14].m_pid=(U32)(PID_TIMER_IPROC);
	g_proc_table[14].m_priority = HIGH;
	g_proc_table[14].m_stack_size=0x100;
	g_proc_table[14].mpf_start_pc = &timer_i;
	
	g_proc_table[15].m_pid=(U32)(PID_UART_IPROC);
	g_proc_table[15].m_priority = HIGH;
	g_proc_table[15].m_stack_size=0x100;
	g_proc_table[15].mpf_start_pc = &uart_i;
}

void timer_i(){
	//for each msg in the delay queue, check their delayed time, pick the ones that have expired, and directly send those messages to target processes
	while(hasExpired(getMsgDelayQueue()) == 1){
		dispatch_delayed_message(); // will be pretty same as regular send
	}
}

//the following are all place holders that will either be implemented shortly after or in P3
void setPriorityProcess(){
	//P3
	k_release_processor();
}

void wallClockDisplay(){
	k_release_processor();
}

void kcd(){
	k_release_processor();
}

void crt(){
	k_release_processor();
}

void uart_i(){
	k_release_processor();
}

void ato_on(void){
	__enable_irq();
}

void ato_off(void){
	__disable_irq();
}

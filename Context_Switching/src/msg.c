#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "msg.h"
#include "k_process.h"
#include "Timer.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

MSG_BUF* delayq;
MSG_BUF* last;

void send_message(int process_id, void *message_envelope){
	PCB* gp_current_process = getcurrentProcess();
	PCB** allPcbs = getgp_pcbs();
	PCB* receiving_proc = allPcbs[process_id];
	int current_priority = gp_current_process->m_priority;
	
	dispatch_message(process_id, gp_current_process->m_pid, message_envelope);
	
	if(receiving_proc->m_state == BLOCKED_ON_RECEIVE){
		receiving_proc->m_state = RDY;
		pushToReadyQ(receiving_proc);
		// if preemption happens
		if(receiving_proc->m_priority > current_priority){
			k_release_processor();		
		}		
	}	
}

void delayed_send(int process_id, void *message_envelope, int delay){
	PCB **gp_pcbs = getgp_pcbs();
	PCB * gp_current_process = getcurrentProcess();
	
	int systemTime = getTimerCount();

	MEM_BLK* mem_blk = get_memory_blk_from_message(message_envelope);
	mem_blk->sendTime = systemTime + delay;
	mem_blk->send_id = gp_current_process->m_pid;
	mem_blk->dest_id = process_id;
	mem_blk->nextMsg = NULL;
	
	addToDelayQueue((MSG_BUF*)message_envelope);
}

void* receive_message(int *sender_id){
	struct msgbuf *msg;
	struct msgbuf *newqueue;
	MEM_BLK* msg_mem_blk = NULL;
	
	PCB * gp_current_process = getcurrentProcess();
	while(gp_current_process->msgqueue==NULL){
		gp_current_process->m_state=BLOCKED_ON_RECEIVE;
		k_release_processor();
	}
	
	//Dequeue
	msg_mem_blk = get_memory_blk_from_message(gp_current_process->msgqueue);
	newqueue = msg_mem_blk->nextMsg;
	
	msg = gp_current_process->msgqueue;
	gp_current_process->msgqueue = newqueue;
	
	if (sender_id != NULL){
		*sender_id = msg_mem_blk->send_id;
	}
	
	msg_mem_blk->send_id = 0;
	msg_mem_blk->dest_id = 0;
	msg_mem_blk->nextMsg = NULL;
	
	return msg;
}

MEM_BLK* get_memory_blk_from_message(void* message){
	MEM_BLK* msg_mem_blk = (MEM_BLK*)((MEM_BLK*)message - sizeof(MEM_BLK));
	return msg_mem_blk;
}

void dispatch_message(int dest_id, int sender_id, void *message_envelope){
	PCB **gp_pcbs = getgp_pcbs();
	MSG_BUF* pre= NULL;
	MEM_BLK* pre_mem_blk = NULL;
	
	//here the message_envelope is the ptr to the memory space
	//find its corresponding memory block, set the header
	MEM_BLK* mem_blk = get_memory_blk_from_message(message_envelope);
	
	
	MSG_BUF* envelope = (MSG_BUF *)message_envelope;
	PCB *receiving_proc = gp_pcbs[dest_id];
	
	// set envelope
	mem_blk->dest_id = dest_id;
	mem_blk->send_id = sender_id;
	mem_blk->nextMsg = NULL;
	
	// enqueue message
	pre = receiving_proc->msgqueue;
	if (pre == NULL){
		receiving_proc->msgqueue = envelope;
	}
	else{
		pre_mem_blk = get_memory_blk_from_message(pre);
		while (pre_mem_blk->nextMsg != NULL){
			pre = pre_mem_blk->nextMsg;
			pre_mem_blk = get_memory_blk_from_message(pre);
		}
		pre_mem_blk->nextMsg = message_envelope;
	}
}


void dispatch_delayed_message(){
	MSG_BUF* toSendMessage = popFromDelayQueue();
	MEM_BLK* mem_blk = get_memory_blk_from_message(toSendMessage);
	
	dispatch_message(mem_blk->dest_id, mem_blk->send_id, toSendMessage);
	
	//TODO
	//potentially enable preemtive scheduling
}


int hasExpired(MSG_BUF* msg){
	 //TODO
	return 1;
}

extern MSG_BUF* getMsgDelayQueue(){
	return delayq;
}

void addToDelayQueue(MSG_BUF* envelope){
	MEM_BLK* mem_blk = get_memory_blk_from_message(envelope);
	int sendTime = mem_blk->sendTime;
	if(delayq == NULL){
		delayq = envelope;	
		last = envelope;
	}else{
		MSG_BUF* ptr = delayq;
		MSG_BUF* pre = NULL;
		MEM_BLK* ptr_blk = get_memory_blk_from_message(ptr);
		//TODO investigae sequence of messages with same send time
		while(ptr != NULL && ptr_blk->sendTime <= sendTime){
			pre = ptr;
			ptr = ptr_blk->nextMsg;			
			ptr_blk = get_memory_blk_from_message(ptr);
		}
		if(ptr == NULL){
			//add at the end
			MEM_BLK* last_blk = get_memory_blk_from_message(last);
			last_blk->nextMsg = envelope;			
			last = envelope;
		}else if(pre == NULL){
			//add at the front
			mem_blk->nextMsg = delayq;
			delayq = envelope;			
		}else{
			//add at the middle
			MEM_BLK* pre_blk = get_memory_blk_from_message(ptr);
			pre_blk->nextMsg = envelope;
			mem_blk->nextMsg = ptr;
		}
	}
}

MSG_BUF* popFromDelayQueue(){
	MSG_BUF* returnQ = NULL;
	
	if(delayq == NULL){
		return delayq;
	}else{
			MEM_BLK* mem_blk = get_memory_blk_from_message(delayq);
			MSG_BUF* newQ = mem_blk->nextMsg;
			returnQ = delayq;
			mem_blk->nextMsg = NULL;
			delayq = newQ;
	}		
	
	if(delayq == NULL)
		last = delayq;
	return returnQ;
	
}

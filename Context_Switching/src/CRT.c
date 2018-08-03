#include "CRT.h"
#include "msg.h"
#include "uart_polling.h"
#include "k_memory.h"

void crt_readMessage(){
	int send_id =-1;
	unsigned char* s = NULL;
	MSG_BUF* msg = NULL;
	MEM_BLK* envelope = NULL;
	// receive message
	msg = receive_message(&send_id);	
	if(msg->mtype==CRT_DIS){
		s = (unsigned char*)&(msg->mtext[0]);
		// release message envelope memory
		envelope = get_memory_blk_from_message(msg);
		k_release_memory_block(envelope);
		// print on console
		uart_put_string(0, s);
	}	
}

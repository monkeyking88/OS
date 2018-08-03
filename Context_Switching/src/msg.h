#ifndef MSG_H_
#define MSG_H_

#include "memoryBlock.h"

typedef struct msgbuf{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} MSG_BUF;


void send_message(int process_id, void *message_envelope);
void delayed_send(int process_id, void *message_envelope, int delay);
void *receive_message(int *sender_id);
MEM_BLK* get_memory_blk_from_message(void* message);

void addToDelayQueue(MSG_BUF* envelope);
MSG_BUF* popFromDelayQueue(void);
MSG_BUF* getMsgDelayQueue(void);


void dispatch_message(int dest_id, int sender_id, void *message_envelope);
void dispatch_delayed_message(void);
int hasExpired(MSG_BUF* msg);



#endif /* ! MSG_H_ */

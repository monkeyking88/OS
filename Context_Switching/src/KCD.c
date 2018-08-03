#include "KCD.h"
#include "k_memory.h"
const int commandNum = 32;
const int processNum = 32;

char* commandTable[processNum][commandNum];

void initializeCommandTable(){
	int i,k;
	for(i = 0; i < processNum; i++){
		for(k = 0; k < commandNum; k++){
			commandTable[i][k] = NULL;
		}
	}
}

void kcd_readMessage(){
	int sender_id =-1;
	MSG_BUF* msg = NULL;
	MEM_BLK* envelope = NULL;
	// receive message
	msg = receive_message(&sender_id);
	// command registration
	if(msg->mtype == KCD_REG){
		addToCommandTable(sender_id,msg->mtext);	
		// release message envelope memory
		envelope = get_memory_blk_from_message(msg);
		k_release_memory_block(envelope);
	}
	// keyboard input
	else if(msg->mtype == DEFAULT){
		if(sizeof(msg->mtype)>=3){
			if(msg->mtext[0]=='%'){
				// look for registered command requester in command table
				notifyCommandRequester(msg);
			}else{
				// if not a command, send the message to CRT
				msg->mtype = CRT_DIS;
				send_message(PID_CRT,msg);
			}			
		}
		else {
			msg->mtype = CRT_DIS;
			send_message(PID_CRT,msg);
		}
	}
}

void addToCommandTable(int procId, char* command){
	//use flag to detect whether we already have the command registed
	int flag = 0;
	int i;
	for(i = 0; i < commandNum && commandTable[procId][i]!=NULL; i++){
		if(stringEqual(commandTable[procId][i],command)){
			flag = 1;
			break;
		}
	}
	
	if(!flag){
		commandTable[procId][i] = command;		
	}
		
}

int clockCommands(char* s){
 return stringEqual(s,"%WR") || stringEqual(s,"%WT") || stringEqual(s,"%WS"); 
}
void notifyCommandRequester(MSG_BUF* msg) {
	int i,j;
	for (i = 0; i < processNum; i++) {
		j = 0;
		while (commandTable[i][j] != NULL) {
			if (validCommand(&(msg->mtext[0]), commandTable[i][j])) {
				// if the input matches any of the registered command
				// send the message to the corresponding process			
				if(clockCommands(commandTable[i][j])){
					//WALK CLOCK COMMAND
					msg->mtype = WALK_CLK_CMD;									
				}
					
				send_message(i,msg);			
				break;
			}
			j++;
		}
	}
}

// check if two char array are equal
// 1 is true, 0 is false
int stringEqual(char* a, char* b) {
		int charsize = sizeof (char);
		char* aptr = a;
		char* bptr = b;
		while(*aptr !='\0' && *bptr !='\0'){
			if(*aptr != *bptr)
				return 0;
			aptr += charsize;
			bptr += charsize;
		}
		return (*aptr)==(*bptr);
}

int validCommand(char* mtext, char* com) {
	int charsize = sizeof (char);
	char* mptr = mtext;
	char* cptr = com;
	int flag = 0;
	while(*mptr !=' '&&!flag){
		if(*mptr != *cptr){
			flag = 1;
		}
		mptr += charsize;
		cptr += charsize;
	}
	
	return flag==1;

}

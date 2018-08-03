#include "wall_clock.h"

int hr = 0;
int min = 0;
int sec = 0;

int terminate = 1;
int set = 0;// set by %ws command
int time = 0;
int pre = 0;

void wall_clock_readMessage(){
	//%WR
	//%WS hh:mm:ss (This one needs checking)
	//%WT
	int send_id =-1;	
	MSG_BUF* msg = NULL;
	MEM_BLK* envelope = NULL;
	// receive message
	msg = receive_message(&send_id);	
	if(msg->mtype==WALK_CLK_CMD){		
		processWalkInstructions(msg->mtext);
		display(msg);
		// release message envelope memory
		envelope = get_memory_blk_from_message(msg);
		k_release_memory_block(envelope);		
	}else if(msg->mtype==WALK_CLK_Increment){
		// increment 1 sec
		time += 1;
		display(msg);
		// release message envelope memory
		envelope = get_memory_blk_from_message(msg);
		k_release_memory_block(envelope);		
	}	
}

void Reset(){
	terminate = 0;
	set = 0;
	time = 0;
	pre = time;
}

void processWalkInstructions(char* instr){
	if(stringEqual(instr,"%WR")){
		Reset();
	}else if(stringEqual(instr,"%WT")){
		terminate = 1;
	}else{
		if(vaildInstr(instr)){
				setTime(instr);
			}
	}	
}

int isNum(char c){
	return '0'<=c && c<='9';
}

int vaildInstr(char* com){
// check %WS hh:mm:ss
	if(com[3]!=' ' || com[6]!=':' || com[9]!=':'){
		return 0;
	}
	if(isNum(com[4])&&isNum(com[5])&&isNum(com[7])&&isNum(com[8])&&isNum(com[10])&&isNum(com[11])){
		return 1;
	}else return 0;
}

void setTime(char* com){
	 hr = (com[4]-'0')*10 + (com[5]-'0');
	 min = (com[7]-'0')*10 + (com[8]-'0');
	 sec = (com[10]-'0')*10 + (com[11]-'0'); 
	
	time = hr*3600 + min*60 + sec;
	pre = time;
	set = 1;
	terminate = 0;
}

void getTime(MSG_BUF* msg){
			int gap = time - pre;
			gap = time - pre;
			hr = gap/3600;
			min = (gap - hr*3600)/60;
			sec = gap - hr*3600 - min*60;
			if(hr < 10){
				msg->mtext[4]='0';
				msg->mtext[5]='0'+(hr-0);
			}else{
				msg->mtext[4]='0'+(hr/10);
				msg->mtext[5]='0'+(hr-(hr/10)*10);
			}
			if(min < 10){
				msg->mtext[7]='0';
				msg->mtext[8]='0'+(min-0);
			}else{
				msg->mtext[7]='0'+(min/10);
				msg->mtext[8]='0'+(min-(min/10)*10);
			}
			if(sec < 10){
				msg->mtext[10]='0';
				msg->mtext[11]='0'+(sec-0);
			}else{
				msg->mtext[10]='0'+(sec/10);
				msg->mtext[11]='0'+(sec-(sec/10)*10);
			}
			pre = time;	
}

void display(MSG_BUF* msg){
	if(!terminate){
		if(!set){
			getTime(msg);
		}
			set = 0;
			msg->mtype = CRT_DIS;
			send_message(PID_CRT,msg);
			msg->mtype = WALK_CLK_Increment;
			delayed_send(PID_CLOCK, msg, 1000);
		
	}
	
}



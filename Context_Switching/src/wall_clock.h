#ifndef WALL_CLOCK_H_
#define WALL_CLOCK_H_
#include "msg.h"
#include "KCD.h"
#include "k_memory.h" 

extern void wall_clock_readMessage(void);
void Reset(void);
void processWalkInstructions(char* instr);
int isNum(char c);
int vaildInstr(char* com);
void setTime(char* com);
void getTime(MSG_BUF* msg);
void display(MSG_BUF* msg);
#endif /* ! WALL_CLOCK_H_ */

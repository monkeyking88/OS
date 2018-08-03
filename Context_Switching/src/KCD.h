#ifndef KCD_H_
#define KCD_H_
#include "msg.h"

/* Message Types */
#define DEFAULT 0
#define KCD_REG 1
#define CRT_DIS 2
#define WALK_CLK_CMD 3
#define WALK_CLK_Increment 4

extern void initializeCommandTable(void);
extern int stringEqual(char* a, char* b);
void kcd_readMessage(void);
void addToCommandTable(int procId, char* command);
void notifyCommandRequester(MSG_BUF* msg);
int validCommand(char* mtext, char* com);
#endif /* ! KCD_H_ */


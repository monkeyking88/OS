/** 
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "Timer.h"

void k_rtx_init(void)
{
				volatile uint8_t sec = 0;
	
        __disable_irq();
				timer_init(0);
        uart0_init();  
	
        memory_init();
        process_init();
        __enable_irq();
	
	/* start the first process */
	
        k_release_processor();
}

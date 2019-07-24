#include "pit.h"

#define DIVIDER_VALUE 1193180
#define CMD_BYTE 0x36
#define HIGH_BYTE 8
#define LOW_BYTE 0xFF
/*
	init_pit
	INTPUT: int frequency
	OUTPUT: none
	SIDE EFFECT: Initializes the pic
*/
void init_pit(int frequency){
	/* Code retrieved from http://www.osdever.net/bkerndev/Docs/pit.htm */
	num_ticks = 0;
	arr_index = 0;

	int divider = DIVIDER_VALUE/frequency;			//Set frequency

	outb(CMD_BYTE, MODE_COMMAND);
	outb(divider & LOW_BYTE, CHANNEL_0);				//Initialize the data ports
	outb(divider >> HIGH_BYTE, CHANNEL_0);
}


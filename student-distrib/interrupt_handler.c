#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "pit.h"
#include "syscall.h"
#include "scheduler.h"
#include "vidctl.h"

#define A_REG 0x8A
#define OUTPORT 0x70
#define INPORT 0x71

#define RATE_FLAG 0x0F
#define PREV_FLAG 0xF0

#define RANDOM_INPUT 1000
#define RANDOM_INPUT_2 0xffffffff
#define TERMBUF_SIZE 127

#define IRQ8 8
#define IRQ2 2
#define IRQ0 0

#define C_REG 0x0C

int seconds = 1;
/*
	interrupt keyboard
	display keys on to the screen
	contains a wrapper function
	in assembly
*/
void interrupt_keyboard(void){

	unsigned char scancode = getchar();			//Receive keyboard input
	
	if(scancode == '\0'){

	}
	else{
		if(visible_screen == 0){						//Throw scancode onto the screen, and store into terminal buffer
			putc_keyboard(scancode);
			terminal1_buffer[counter1] = scancode;
			counter1++;
		}
		else if(visible_screen == 1){
			putc_keyboard(scancode);	
			terminal2_buffer[counter2] = scancode;
			counter2++;
		}
		else if(visible_screen == 2){
			putc_keyboard(scancode);	
			terminal3_buffer[counter3] = scancode;		//Depending on which screen, use different terminal buffers
			counter3++;
		}
	}

	if(scancode == '\n' || counter1 >= TERMBUF_SIZE || counter2 >= TERMBUF_SIZE || counter3 >= TERMBUF_SIZE){           //Overflow handling, if either enter is pressed or buffer is full, set flag to 1
		flag_keyboard[visible_screen] = 1;
	}
	send_eoi(1);
}
/*
	interrupt keyboard
	display keys on to the screen
	contains a wrapper function
	in assembly
*/
void interrupt_rtc(void){

	flag_rtc = 1;

	rate &= RATE_FLAG;
	
	outb(A_REG, OUTPORT);                           //Select Register A
	char prev1 = inb(INPORT);                       //Read from INPORT
	outb(A_REG, OUTPORT);
	outb((prev1 & PREV_FLAG) | rate, INPORT);		//Changing the frequency of RTC, obtained from obtained from https://wiki.osdev.org/RTC
		
	//test_interrupts();								//RTC test, provided in lib.c
	
	outb(C_REG, OUTPORT);							// select register C, to continously receive interrupts
	inb(INPORT);
		
	send_eoi(IRQ8);									//Send EOI 
	send_eoi(IRQ2);
}
/*
	interrupt pit
	display keys on to the screen
	contains a wrapper function
	in assembly
*/
void interrupt_pit(void){
	/* Retrieved from http://www.osdever.net/bkerndev/Docs/pit.htm */
	num_ticks++;
	send_eoi(IRQ0);
	context_switch();                               //Send EOI before calling context_switch as PIT does not get reset
	
}
/*
	interrupt master 
	display keys on to the screen
	contains a wrapper function
	in assembly
*/
void interrupt_master(void){
	send_eoi(0);
}
/*
	interrupt slave
	display keys on to the screen
	contains a wrapper function
	in assembly
*/
void interrupt_slave(void){
	send_eoi(2);
}


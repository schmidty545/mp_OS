/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */
#include "i8259.h"
#include "lib.h"
#define ZERO 0x00

#define EIGHT 8

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */
/* Initialize the 8259 PIC */

/*
	i8259 init
	INPUT: none
	OUTPUT: none
	SIDE EFFECTS: initialize the 8259 PIC
*/
void i8259_init(void) {												//The following code has been written with reference to OSDEV.org
 outb(ICW1, MASTER_8259_PORT);
 outb(ICW1, SLAVE_8259_PORT);										//Initialize the master/slave ports
 
 outb(ICW2_MASTER, MASTER_8259_PORT + 1);
 outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);								//Initialize their data ports
 
 outb(ICW3_MASTER, MASTER_8259_PORT + 1);
 outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);								//Perform cascade
 
 outb(ICW4, MASTER_8259_PORT + 1);
 outb(ICW4, SLAVE_8259_PORT + 1);									
}
/* Enable (unmask) the specified IRQ */
/*
	enable irq
	INPUT: irq_num
	OUTPUT: none
	SIDE EFFECTS: set mask for specific interrupt
*/
void enable_irq(uint32_t irq_num) {
 short port;
 char mask;
 
 if(irq_num < EIGHT){
  port = MASTER_8259_PORT + ICW4;
 }
 else{
  port = SLAVE_8259_PORT + ICW4;									//Identify which PIC to use
  irq_num -= EIGHT;
 }
 mask = inb(port) & ~(1 << irq_num);								//recover mask from port and perform AND as is low active
 outb(mask, port);
}
/* Disable (mask) the specified IRQ */
/*
	disable irq
	INPUT: irq_num
	OUTPUT: none
	SIDE EFFECTS:  unmask for specific interrupt
*/
void disable_irq(uint32_t irq_num) {
 short port;
 char mask;
 
 if(irq_num < EIGHT){
  port = MASTER_8259_PORT + ICW4;
 }
 else{
  port = SLAVE_8259_PORT + ICW4;
  irq_num -= EIGHT;
 }
 
 mask = inb(port) | 1 << irq_num;									//As with unmask, logical OR as it is low active
 outb(mask, port);
}
/* Send end-of-interrupt signal for the specified IRQ */
/*
	send_eoi
	INPUT: irq_num
	OUTPUT: none
	SIDE EFFECTS: send EOI signal to the PIC
*/
void send_eoi(uint32_t irq_num) {
 if(irq_num >= EIGHT){
  outb(EOI | (irq_num - EIGHT), SLAVE_8259_PORT);
 }
 else{
  outb(EOI | irq_num, MASTER_8259_PORT);							//Depending on the IRQ number, send EOI to either slave or master PIC
 }
}


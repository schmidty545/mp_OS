#include "x86_desc.h"
#include "syscall.h"

#define TOTAL_ENTRY 256
#define EXCEPTION_SYS 255
#define SYS_CALL 128
#define KEYBOARD 0x21
#define RTC 0x28
#define PIT 0x20
#define THIRTYFOUR 34
#define NINE 9
#define FIFTEEN 15
#define THIRTYONE 30
#define MASTER 0x20
#define SLAVE 0x22

extern void key_wrapper();
extern void rtc_wrapper();
extern void general_wrapper();
extern void master_wrapper();
extern void slave_wrapper();
extern void interrupt_wrapper();
extern void syscall_wrapper();
extern void pit_wrapper();

/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void divide_by_zero(){
	printf("you can't divide by zero bimbo\n");			//Print which exception has been generated
	cli();												//Mask out all interrupts
	halt_exception();									//Call HALT, returning an exception specific number
	while(1);											//To freeze the terminal upon exception
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void debug_error(){
	printf("debug error\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void non_maskable_interrupt(){
	printf("non maskable interrupt\n");
	cli();
	halt_exception();
	while(1);
	
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void breakpoint(){
	printf("breakpoint error\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void overflow(){
	printf("overflow error\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void bound_range_excess(){
	printf("bound range excess\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void exception_nine(){
	printf("EXCEPTION 9\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void invalid_opcode(){
	printf("invalid opcode\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void device_unavailable(){
	printf("device unavailable\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void double_fault(){
	printf("double fault\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void invalid_tss(){
	printf("invalid tss\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void segment_not_present(){
	printf("segment not present\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void stack_segment_fault(){
	printf("stack not present\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void general_protection_fault(){
	printf("general_protection_fault\n");
	cli();
	halt_exception();
	while(1);
	
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void page_fault(){
	printf("page_fault\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void floating_point_exception(){
	printf("floating_point_exception\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void alignment_check(){
	printf("alignment_check\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void machine_check(){
	printf("machine_check\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void floating_point_exception_SIMD(){
	printf("floating_point_exception_SIMD\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void virtualization_exception(){
	printf("virtualization_exception\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void security_exception(){
	printf("security_exception\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void triple_fault(){
	printf("triple_fault\n");
	cli();
	halt_exception();
	while(1);
}
/*
	Helper function to print out which
	exception has been generated
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: displays exception
*/
void syscall_helper(){
	printf("system call received\n");
	
}
/*
	
	initialize_idt
	INPUT : none
	OUTPUT: none
	SIDE EFFECT: Populates the interrupt descriptor table
*/
void initialize_idt(){

	 int i;
	 
	 for(i = 0; i < TOTAL_ENTRY; i++){     	//Initially, just set everything to 0.
	  idt[i].dpl = 0;
	  idt[i].present = 1;
	  idt[i].seg_selector = KERNEL_CS;		//From the doc, all seg selectors to be set to kernel code segment
	  
	  idt[i].size = 1;
	  
	  idt[i].reserved0 = 0;
	  idt[i].reserved1 = 1;
	  idt[i].reserved2 = 1;
	  idt[i].reserved3 = 0;
	  idt[i].reserved4 = 0;       			//Just conventions, reserved3 should be 1 for exceptions, 0 for interrupts
	  
	  if(i != NINE || i != FIFTEEN || i != THIRTYONE){		//Reserved entries
	   //idt[i].present = 1;       			//Because the first 20 indices are occupied with exceptions, mark as present
	   idt[i].reserved3 = 1;      			//This bit indicates that it is an exception. 20 exceptions
	  }
	  if((i == KEYBOARD) || (i==RTC) || (i==MASTER) || (i==SLAVE) || (i==PIT)){
	   idt[i].present = 1;
	   idt[i].reserved3 = 0; 				//For RTC and keyboard interrupts, set reserved3 as 0		
	  }
	  if(i == SYS_CALL){
	   idt[i].present = 1;
	   idt[i].dpl = 3;        //Specific to system calls
	  }
	 }
	 
	 SET_IDT_ENTRY(idt[0], divide_by_zero);
	 SET_IDT_ENTRY(idt[1], debug_error);
	 SET_IDT_ENTRY(idt[2], non_maskable_interrupt);
	 SET_IDT_ENTRY(idt[3], breakpoint);
	 SET_IDT_ENTRY(idt[4], overflow);
	 SET_IDT_ENTRY(idt[5], bound_range_excess);
	 SET_IDT_ENTRY(idt[6], invalid_opcode);
	 SET_IDT_ENTRY(idt[7], device_unavailable);
	 SET_IDT_ENTRY(idt[8], double_fault);
	 SET_IDT_ENTRY(idt[9], exception_nine);
	 SET_IDT_ENTRY(idt[10], invalid_tss);
	 SET_IDT_ENTRY(idt[11], segment_not_present);
	 SET_IDT_ENTRY(idt[12], stack_segment_fault);
	 SET_IDT_ENTRY(idt[13], general_protection_fault);
	 SET_IDT_ENTRY(idt[14], page_fault);
	 SET_IDT_ENTRY(idt[16], floating_point_exception);
	 SET_IDT_ENTRY(idt[17], alignment_check);
	 SET_IDT_ENTRY(idt[18], machine_check);
	 SET_IDT_ENTRY(idt[19], floating_point_exception_SIMD);		
	 SET_IDT_ENTRY(idt[20], virtualization_exception);
	 SET_IDT_ENTRY(idt[30], security_exception);				//Order of exceptions referenced from OSDEV.org
     SET_IDT_ENTRY(idt[RTC], rtc_wrapper);
	 SET_IDT_ENTRY(idt[KEYBOARD], key_wrapper);					//Set all IDT entries according to exceptions and interrupts
	 SET_IDT_ENTRY(idt[MASTER], master_wrapper);				//Also set IDT entries for interrupt devices and syscalls
	 SET_IDT_ENTRY(idt[SLAVE], slave_wrapper);
	 SET_IDT_ENTRY(idt[PIT], pit_wrapper);

	 for(i = 32; i < TOTAL_ENTRY; i++){
		if(i != RTC && i != KEYBOARD && i != MASTER && i != SLAVE && i != SYS_CALL && i != PIT){
			SET_IDT_ENTRY(idt[i], general_wrapper);				//Rather than leaving entries, blank, fill them up with a generic general wrapper
		}
		if(i == SYS_CALL){
			SET_IDT_ENTRY(idt[i], syscall_wrapper);				//Fill the syscall IDT entry
		}
	 }
	 
	 lidt(idt_desc_ptr);
}




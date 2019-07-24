#include "scheduler.h"
#include "vidctl.h"
#include "paging.h"
#include "i8259.h"

#define KERNEL_SPACE 0x800000
#define MEMORY_SPACE 0x400
#define VID_MEMORY 0xb8000
#define FOURKB 0x1000
/*
	init_scheduler
	INTPUT: none
	OUTPUT: none
	SIDE_EFFECTS: set process array to an initial value
*/
int32_t init_scheduler(){
	schedule_index = 0;
	three_shells_executed = 0;					
	
	int i = 0;
	
	for(i = 0; i < 3; i++){
		process_array[i].cur_pid = -1;													//Initially, no parent processes running so set to -1
		process_array[i].cur_fd_ptr=kernel_fdt;								
		process_array[i].cur_esp0=(uint8_t*)(KERNEL_SPACE - (MEMORY_SPACE*i));	
		process_array[i].cur_ebp=(uint8_t*)(KERNEL_SPACE - (MEMORY_SPACE*i));
		process_array[i].cur_esp=(uint8_t*)(KERNEL_SPACE - (MEMORY_SPACE*i));			
	}
	
	return 0;
}

/*
	switch_context
	INPUTS: const uint8_t* command x 2
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
void context_switch(){

	int32_t next_schedule;
	
	if(schedule_index < 2){
		next_schedule = schedule_index + 1;
	}
	else{
		next_schedule = 0;															//Set schedule index accordingly for context switching
	}
	
	asm volatile(
		"mov %%esp, %0;"
		"mov %%ebp, %1;"
		:"=r" (process_array[schedule_index].cur_esp), "=r" (process_array[schedule_index].cur_ebp)				//Save ebp and esp
		:
		:"memory"
	);
		
	process_array[schedule_index].cur_esp0 = (uint8_t*)tss.esp0;												//Save TSS
		
	schedule_index = next_schedule;																				//Update schedule_index	
	pcb_t* pcb_temp = get_pcb(process_array[schedule_index].cur_pid);
	if(visible_screen == schedule_index){																		//If visible_screen == schedule index, map to video memory
		map_screen_buffer(0);	
	}
	else{
		map_screen_buffer(1 + schedule_index);																	//else map to video buffer
	}
	
	tss.esp0 = (uint32_t)process_array[schedule_index].cur_esp0;
	
	
	
	if(pcb_temp->vid_set){
		if(visible_screen != schedule_index){
			pte_vid[0] = (PAGE_KB+(VID_MEMORY+(FOURKB*(schedule_index + 1))))|5;								//Map to buffer
		}
		else if(visible_screen == schedule_index){
			pte_vid[0] = (PAGE_KB+VID_MEMORY)|5;																//Map to video memory
		}
	}
	else
		pte_vid[0] = 0;	

	asm volatile(
		"mov %0, %%esp;"
		"mov %1, %%ebp;"
		:
		:"r" (process_array[schedule_index].cur_esp), "r" (process_array[schedule_index].cur_ebp)				//Overwrite esp and ebp
		:"esp", "ebp"
	);
	
	if(process_array[schedule_index].cur_pid != -1){
		map_page_exec(process_array[schedule_index].cur_pid);													//Set paging for the current process in context switch
	}
	else{
		boot_shell();																							//Helper function to execute shell
	}
	
	asm volatile("mov %0, %%eax;"
		"mov %%eax, %%cr3;"
		:
		:"r"(pdt)
		:"eax", "memory"																					//TLB Flushing
	);

	return;
}
/*
	boot_shell
	INTPUT: none
	OUTPUT: none
	SIDE_EFFECTS: executes shell
*/
void boot_shell(){
	while(1){
		execute((const uint8_t*)"shell");																		//Executes shell
	}
}


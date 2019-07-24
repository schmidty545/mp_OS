#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "lib.h"
#include "process.h"
#include "syscall.h"
#include "x86_desc.h"
#include "filesys.h"

/* Global variables to keep track of processes*/
uint8_t schedule_index;
uint32_t three_shells_executed;

/* Process array struct */
typedef struct{
	uint32_t cur_pid;
	uint8_t* cur_esp0;
	uint8_t* cur_esp;
	uint8_t* cur_ebp;	
	fd_t* cur_fd_ptr;
}switch_table_t;

switch_table_t process_array[3];

/* Initialization function */
int32_t init_scheduler();
/* Function to switch contexts */
void context_switch();
/* This is a helper function to reboot the shell */
void boot_shell();

#endif


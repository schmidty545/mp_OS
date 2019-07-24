#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "types.h"
#include "filesys.h"
/* Process Control Block struct*/
typedef struct{
	uint32_t use;
	uint32_t parent_pid;					//Keeps track of it's parent process
	uint8_t* parent_esp0;					//Keeps track of it's parent processe's Kernel esp
	uint8_t* parent_esp;					//Keeps track of it's parent processe's esp
	uint8_t* parent_ebp;					//Keeps track of it's parent processe's ebp
	fd_t* parent_fd;						//Keeps track of it's parent processe's file descriptor
	fd_t fd_table[8];
	uint8_t user_buf[1024];					//Used for getargs function, for parsing arguments
	uint32_t vid_set;						//Used to indicate screen switching
	uint32_t screen_idx;					//Keeps track of which screen it is currently displayed
}pcb_t;
/* Initialization function*/
void process_init();
/* Initialization function*/
void pcb_init(uint32_t idx);
/* Getting the index function*/
int32_t get_avail_pcb();
/* Getting the PCB function*/
pcb_t* get_pcb(uint32_t idx);
/* Getting the esp function*/
void* get_esp0(uint32_t idx);

#endif


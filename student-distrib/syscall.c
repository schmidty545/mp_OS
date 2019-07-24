#include "syscall.h"
#include "filesys.h"
#include "paging.h"
#include "process.h"
#include "lib.h"
#include "x86_desc.h"
#include "vidctl.h"
#include "scheduler.h"

#define TARGET_MEM 0x08048000
#define ENTRY_MEM 0x08048018
#define STACK_POINTER 0x083ffffc
#define RANDOM 128
#define FOUR 4
#define MAGIC_CONST1 0x7f
#define MAGIC_CONST2 0x45
#define MAGIC_CONST3 0x4c
#define MAGIC_CONST4 0x46
#define EXCEPTION_RET 256
#define USR_BUF_SIZE 1024
#define MAX_FD 7
#define MIN_FD 2
/*
	open
	INPUTS: const uint8_t* filename
	OUTPUTS: fd
	SIDE_EFFECTS: opens file according to their file names and reads
	its contents
*/

int32_t open(const uint8_t* filename)
{
	int fd=get_avail_fd();													//get index of descriptor tables
	dentry_t entry;
	
	if(fd==-1)
		return -1;															//Error check
	if(read_dentry_by_name(filename, &entry)==-1)
		return -1;										
	
	//garbage value checking
	set_fileop_ptr(fd, entry.type);											//Set the fileop pointer according to the entry type(rtc, keyboard etc)
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer->open(fd, entry.inode);				//Initialize file descriptor and return fd
	return fd;
	
}
/*
	write
	INPUTS: int32_t fd, const void* buf, int32_t nbytes
	OUTPUTS: file descriptor
	SIDE_EFFECTS: returns the exact file descriptor from the descriptor table
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
	if(fd < 0 || fd > MAX_FD){															//Make sure fd is not out of the file_descriptor bound
		return -1;
	}
	if(buf == NULL){																//If buf is NULL, that is to return -1
		return -1;
	}
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)						//Error check, if flag is 0, return -1
		return -1;
	if(fd==0)
		return -1;
	return process_array[schedule_index].cur_fd_ptr[fd].table_pointer->write(fd, buf, nbytes);
}
/*
	read
	INPUTS: int32_t fd, const void* buf, int32_t nbytes
	OUTPUTS: 0
	SIDE_EFFECTS: returns the exact file descriptor from the descriptor table
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
	if(fd < 0 || fd > MAX_FD){															//Make sure fd is not out of the file_descriptor bound
		return -1;
	}
	
	if(buf == NULL){																//If buf is NULL, that is to return -1
		return -1;
	}
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)						//Error check, if flag is 0, return -1
		return -1;
	if(fd==1)
		return -1;
	return process_array[schedule_index].cur_fd_ptr[fd].table_pointer->read(fd, buf, nbytes);
}
/*
	close
	INPUTS: int32_t fd
	OUTPUTS: 0
	SIDE_EFFECTS: returns the exact file descriptor from the descriptor table
*/
int32_t close(int32_t fd)
{
	if(fd < MIN_FD || fd > MAX_FD){
		return -1;
	}
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)						//Error check, if flag is 0, return -1
		return -1;
	
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer->close(fd);
	return 0;
}
/*
	execute
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t halt_exception(void){
	cli();
	
	int i;
	
	pcb_t* pcb=get_pcb(process_array[schedule_index].cur_pid);						//Get current PCB
	tss.esp0=(uint32_t)pcb->parent_esp0;									//Assign esp to parent PCB
	map_page_exec(pcb->parent_pid);
	clear_vid();
	
	for(i = 0; i < 8; i++){
		close(i);
	}
	
	process_array[schedule_index].cur_pid = pcb->parent_pid;
	process_array[schedule_index].cur_fd_ptr=pcb->parent_fd;								//Restore parent data
	pcb->use=0;													//Mark PCB as not in use
	asm volatile(
		"mov %0, %%esp;"
		"mov %1, %%ebp;"
		"sti;"
		"jmp exec_ret;"											//Jump back to execute
		:
		:"r" (pcb->parent_esp), "r" (pcb->parent_ebp)
		:"esp", "ebp"
	);
	ret_status = EXCEPTION_RET;									//If exception is generated, return 256
	
	return EXCEPTION_RET;
}
/*
	halt
	INPUTS: const uint8_t status
	OUTPUTS: NONE
	SIDE_EFFECTS: restores parent data and returns to execute iret
*/
int32_t halt(uint8_t status){
	cli();
	
	int i;
	
	ret_status=(int8_t)status;
	pcb_t* pcb=get_pcb(process_array[schedule_index].cur_pid);						//Get current PCB
	tss.esp0=(uint32_t)pcb->parent_esp0;		//Assign esp to parent PCB
	
	process_array[schedule_index].cur_pid = pcb->parent_pid;
	
	clear_vid();
	
	for(i = 0; i < 8; i++){
		close(i);
	}
	
	process_array[schedule_index].cur_fd_ptr=pcb->parent_fd;								//Restore parent data
	pcb->use=0;					//Mark PCB as not in use	
		
	map_page_exec(pcb->parent_pid);
	asm volatile(
		"mov %0, %%esp;"
		"mov %1, %%ebp;"
		"sti;"
		"jmp exec_ret;"											//Jump back to execute
		:
		:"r" (pcb->parent_esp), "r" (pcb->parent_ebp)
		:"esp", "ebp"
	);
	
	return status;
}
/*
	execute
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t execute(const uint8_t* command){
	cli();
	uint8_t fname[RANDOM]={0,};							//Command buffer to store command
	dentry_t dent;
	inode_t* inode=(inode_t*)FILEMEM;
	uint8_t magic[FOUR];
	uint8_t* target=(uint8_t*)TARGET_MEM;
	uint32_t* entry=(uint32_t*)ENTRY_MEM;						//Memory locations to load the program
	uint8_t* esp=(uint8_t*)STACK_POINTER;
	uint8_t* esp0;
	
	pcb_t* pcb;
	uint32_t pcb_idx;
	
	uint32_t i;
	
    if(command[0] != ' ' && command[0] != '\n' && command[0] != '\0') {
        for( i = 0; i < strlen((const char*)command); i++) {
			
			if(command[i] == ' ' || command[i]=='\n'){								//Parse arguments by removing the first arg which is
				fname[i] = 0;														//Separated from the rest from a spacing
                break;
            }
			else
			{
				fname[i] = command[i];
			}
        }
    }
	
	if(read_dentry_by_name(fname, &dent)==-1)										//Check the validity of the argument
		return -1;
	inode+=dent.inode+1;
	
	read_data(dent.inode, 0, magic, FOUR);
	if(magic[0]!=MAGIC_CONST1 || magic[1]!=MAGIC_CONST2 || magic[2]!=MAGIC_CONST3 || magic[3]!=MAGIC_CONST4)		//Check for magic constants
		return -1;

		target=(uint8_t*)TARGET_MEM;
		pcb_idx=get_avail_pcb();
		if(pcb_idx==-1)																	//Check for PCB validity
			return -1;
		pcb=get_pcb(pcb_idx);															//Get current PCB
		pcb->parent_pid=process_array[schedule_index].cur_pid;							//Make the current processe's parent process the process that is within the process array
		process_array[schedule_index].cur_pid = pcb_idx;								//Set the process_array according to the current process running
												
		pcb->use=1;																		//Set current PCB as in use
		pcb->vid_set=0;																	//Set the vid_set field to 0, this will determine if screen switch is necessary	
		
		pcb->parent_fd=process_array[schedule_index].cur_fd_ptr;
		process_array[schedule_index].cur_fd_ptr=pcb->fd_table;							//Update the file_descriptors unique to the current PCBs
		init_fd();
		
		pcb->screen_idx = schedule_index;												//Update the screen_index to the schedule_index
		
		strncpy((char*)pcb->user_buf, (const char*)command, USR_BUF_SIZE);				//Copy the command into a temporary buffer for usage in the function getargs
		
		esp0=get_esp0(pcb_idx);															//Update Kernel esp
		map_page_exec(pcb_idx);															//Update the Page Table

		pcb->parent_esp0=(uint8_t*)tss.esp0;											//Update the PCB's parent esp0
		tss.esp0=(uint32_t)esp0;
		
		read_data(dent.inode, 0, target, inode->length);								
		target=(uint8_t*)(*entry);
		
		asm volatile(
					"mov %%esp, %0;"
					"mov %%ebp, %1;"
					:"=r" (pcb->parent_esp), "=r" (pcb->parent_ebp)						//Store ebp, and esp into the PCB's parent esp and ebp pointers
					:
					:"memory"
					);
					
					
		
		asm volatile(
					"push $0x2b;"
					"push %0;"
					"pushfl;"															//Push EFLAGS for interrupts
					"popl %%eax;"
					"orl $0x200, %%eax;"
					"pushl %%eax;"
					"push $0x23;"
					"push %1;"
					"movw $0x2b, %%ax;"
					"movw %%ax, %%ds;"													
					"iret;"
					"exec_ret:"															//Setting up the IRET, HALT will jump back to execute after its execution
					"sti;"
					:
					:"r" (esp), "r" (target)
					:"memory", "eax");
					
	return ret_status;																	//Return an execution specific number from 0 to 256	
	
}

/*
	getargs
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t getargs(uint8_t* buf, int32_t nbytes){
    	
	int i = 0;
	int j = 0;
	
	for(i = 0; i < USR_BUF_SIZE; i++){
		arg_retrieved[i] = '\0';											//Just put NULL into the array for safety
	}
	
	pcb_t* cur_pcb = get_pcb(process_array[schedule_index].cur_pid);								//Get current process from the PCB struct
	
	int32_t size_of_string = strlen((const char*)cur_pcb->user_buf);		
	
	cur_pcb->user_buf[size_of_string] = '\0';								//Add a NULL character to the end of the argument
	
	if(buf == NULL || nbytes == 0){														//If number of bytes is larger than the size of the buffer
		return -1;															//return -1, because its an error
	}
	
	for(i = 0; i < strlen((const char*)cur_pcb->user_buf); i++){
		if((cur_pcb->user_buf)[i] == ' ' && (cur_pcb->user_buf)[i + 1] != ' '){
			i++;
			break;															//Locate where the space is within the argument
		}
	}

	for(j = 0; j < strlen((const char*)cur_pcb->user_buf); j++, i++){		//Parse the argument
		if((cur_pcb->user_buf)[i] == '\0'){
			break;
		}
		
		arg_retrieved[j] = (cur_pcb->user_buf)[i];
	}
	
	if(arg_retrieved[0] == '\0'){											//Error check
		return -1;
	}
	
	strncpy((int8_t*)buf, (const int8_t*)arg_retrieved, (uint32_t)nbytes);		

	return 0;
}
/*
	vidmap
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t vidmap(uint8_t** screen_start){
	if(screen_start==NULL)
		return -1;
	if((unsigned int)screen_start < 0x800000)					//If input is either a NULL pointer or in Kernel space
		return -1;												//return -1 for error
	pcb_t* pcb=get_pcb(process_array[schedule_index].cur_pid);
	pcb->vid_set=1;												//set video memory as used
	return map_vid(screen_start);								//Call helper function that will map the page table to video memory
}
/*
	set_handler
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t set_handler(int32_t signum, void* handler_address){
	return -1;													//Currently unused, so just return -1
}
/*
	sigreturn
	INPUTS: const uint8_t* command
	OUTPUTS: NONE
	SIDE_EFFECTS: executes any executable files
*/
int32_t sigreturn(void){
	return -1;													//Currently unused, so just return -1
}


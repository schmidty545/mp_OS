#include "process.h"

#define EIGHTK 0x2000
#define ESP_MEM 0x800000
#define PCB_MEM 0x7fe000
#define FOUR 4
#define EIGHT 8
/*
	process_init()
	INTPUT: none
	OUTPUT: none
	Sets all PCBs as free, aka not in use
*/
void process_init()
{
	int i;
	for(i=0;i<EIGHT;i++)
	{
		get_pcb(i)->use=0;				//Set all OCB as not in use
	}
}
/*
	process_init()
	INTPUT: none
	OUTPUT: none
	Sets all PCBs as free, aka not in use
*/
void pcb_init(uint32_t idx)
{
}
/*
	get_avail_pcb()
	INTPUT: none
	OUTPUT: index of PCB
	Returns the index to which PCB is at use, else -1
*/
int32_t get_avail_pcb()
{
	int i;
	for(i=0;i<EIGHT;i++)
	{
		if(get_pcb(i)->use==0)			//Locate the correct PCB in usage
			return i;
	}
	return -1;							//If no match is found, return -1
}
/*
	pcb_t*get_pcb(uint32_t idx)
	INTPUT: uint32_t idx
	OUTPUT: none
	Get the PCB from the kernel stack
*/
pcb_t* get_pcb(uint32_t idx)
{
	return (pcb_t*)(PCB_MEM - (EIGHTK*(idx+1)));			//+1, so as to not corrupt the stack
}
/*
	get_esp0()
	INTPUT: uint32_t idx
	OUTPUT: none
	Return the esp0, which is the TSS element
*/
void* get_esp0(uint32_t idx)
{
	return (void*)(ESP_MEM - (EIGHTK*(idx+1)) -FOUR);			//+1, so as to not corrupt the stack
}


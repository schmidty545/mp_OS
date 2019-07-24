#include "paging.h"
#define FOURMB 0x00400000
#define FOURKB 0x1000
#define START_ADDRESS 0x00800087
#define VIDEO_MEMORY 184
#define VIDEO_OFFSET 0xb8000

#define PAGE_SIZE 1024
/*
	init paging
	initialize the paging function
	INPUT: none
	OUTPUT: none
	SIDE EFFECTS: Align the memory address
*/
void init_paging()
{
	int i;
	for(i=0;i<PAGE_SIZE;i++)
	{
		pdt[i]=PAGE_MB + (FOURMB * i);				//add 4mb * i for the offset of each page	
		pte[i]=PAGE_KB + (FOURKB * i);				//add 4kb * i for the offset of each page
	}
	pdt[0]=((unsigned int)pte) | 1;					//marking the first pdt entry as present, and put the offset to pte
	pdt[1]|=1;										//marking the second pdt entry as present
	pdt[EXEC]=START_ADDRESS;						//mapping virtual 128mb to physical 8mb
	pdt[VID]=((unsigned int)pte_vid);
	pte[VIDEO_MEMORY+1]|=1;
	pte[VIDEO_MEMORY+2]|=1;
	pte[VIDEO_MEMORY+3]|=1;
	pte[VIDEO_MEMORY+4]-=0x4000;
	pte[VIDEO_MEMORY+4]|=1;
	pte[VIDEO_MEMORY]|=1;							//pte[184] is for 0xb8000, which is video memory
	
 
	asm volatile("mov %0, %%eax;"					
				"mov %%eax, %%cr3;"					//put the address of pdt to cr3
	
				"mov %%cr4, %%eax;"
				"or $0x00000010, %%eax;"
				"mov %%eax, %%cr4;"					//enable 4mb paging
	
				"mov %%cr0, %%eax;"
				"or $0x80000000, %%eax;"
				"mov %%eax, %%cr0;"					//enable paging 
				:
				:"r" (pdt)
				:"eax", "memory"
				);
	
}

/*
	map_page_exec
	initialize the paging function
	INPUT: none
	OUTPUT: none
	SIDE EFFECTS: Align the memory address
*/
void map_page_exec(uint32_t idx)
{
//	if(idx<0 || idx > 7)
//		return;
	
	pdt[EXEC]=START_ADDRESS + (FOURMB * (idx));			//Allocate space for program execution
	asm volatile("mov %0, %%eax;"
				"mov %%eax, %%cr3;"
				:
				:"r"(pdt)
				:"eax", "memory"
				);
}
/*
	map vid
	initialize the paging function
	INPUT: none
	OUTPUT: none
	SIDE EFFECTS: Align the memory address
*/
uint32_t map_vid(uint8_t** screen_start)
{
	pdt[VID]=((unsigned int)pte_vid)|7;						//Map the newly created page table, logical OR with attribute bit
	pte_vid[0]=(PAGE_KB+VIDEO_OFFSET) | 5;
	*screen_start=(uint8_t*)(FOURMB*VID);					//Set video memory to display the fish
	
	asm volatile("mov %0, %%eax;"
				"mov %%eax, %%cr3;"
				:
				:"r"(pdt)
				:"eax", "memory"							
				);
	return 1;
}
/*
	clear_vid
	initialize the paging function
	INPUT: none
	OUTPUT: none
	SIDE EFFECTS: Align the memory address
*/
uint32_t clear_vid()
{
	pte_vid[0]=0;							//Clear video memory of	
	asm volatile("mov %0, %%eax;"							//Used frames
				"mov %%eax, %%cr3;"
				:
				:"r"(pdt)
				:"eax", "memory"
				);
	return 1;
}

uint32_t map_screen_buffer(uint32_t idx)
{
	if(idx < 0 || idx > 3)
		return 0;
	
	pte[VIDEO_MEMORY]=(VIDEO_OFFSET+(FOURKB*idx))|3;
	asm volatile("mov %0, %%eax;"							//Used frames
				"mov %%eax, %%cr3;"
				:
				:"r"(pdt)
				:"eax", "memory"
				);
	return 1;
}


#include "vidctl.h"
#include "paging.h"
#include "scheduler.h"
#define VID_MEMORY 0xb8000
#define BUFFER_1 0xb9000
#define BUFFER_2 0xba000
#define BUFFER_3 0xbb000
#define FOURKB 0x1000
#define ATTRIBUTE_BITS 0x7
#define KEY_VID 0xbc000
#define FOURKB 0x1000
#define BYTES_COPY 4000
#define VID_SIZE 2000
/*
	init_vid
	INTPUT: none
	OUTPUT: none
	SIDE EFFECTS: Initialize video memory with the attribute bits
*/
void init_vid()
{
	int i;
	for(i=0;i<VID_SIZE;i++)											
	{
		((uint8_t*)BUFFER_1)[i*2]=0x0;
		((uint8_t*)BUFFER_1)[(i*2)+1]=ATTRIBUTE_BITS;				
		((uint8_t*)BUFFER_2)[i*2]=0x0;
		((uint8_t*)BUFFER_2)[(i*2)+1]=ATTRIBUTE_BITS;					//In the video memory stack, initialize all buffers to hold
		((uint8_t*)BUFFER_3)[i*2]=0x0;						//the attribute bits, which is 7
		((uint8_t*)BUFFER_3)[(i*2)+1]=ATTRIBUTE_BITS;
		//((uint8_t*)0xbc000)[i*2]=0x0;
		//((uint8_t*)0xbc000)[(i*2)+1]=0x7;
	}
	visible_screen=0;										//Initially, target screen is screen 0
}
/*
	copy_vid
	INTPUT: int32_t target screen
	OUTPUT: none
	SIDE EFFECTS: copies video memory into buffer
*/
uint32_t copy_vid(int32_t target_screen)
{
	if(target_screen<-1 || target_screen >2)
		return 0;
	
	return 0;
}
/*
	switch_vid
	INTPUT: int32_t target_screen
	OUTPUT: 0 on failure, 1 on success
	SIDE EFFECTS: switch screens on ALT + fn from 1 to 3
*/
uint32_t switch_vid(int32_t target_screen)
{
	if(target_screen < -1 || target_screen > 2)					//Error check
		return 0;
	if(target_screen==visible_screen)							//Cannot switch to the same screen
		return 1;
	
	//uint32_t i;
	uint32_t write=BUFFER_1 + (FOURKB * target_screen);
	uint32_t save=BUFFER_1 + (FOURKB * visible_screen);			//Add 4kb to the video memory
	
	
	
	/*
	for(i=0;i<4000;i++)
	{
		((uint8_t*)save)[i]=((uint8_t*)0xb8000)[i];
		((uint8_t*)0xb8000)[i]=((uint8_t*)write)[i];
	}
	*/

	memcpy((void*)save, (void*)KEY_VID, BYTES_COPY);
	memcpy((void*)KEY_VID, (void*)write, BYTES_COPY);					//Display buffer onto the video memory and set target screen
	visible_screen=target_screen;
	
	if(visible_screen!=schedule_index)
	{
		pte_vid[0] = (PAGE_KB+(VID_MEMORY+(FOURKB*(schedule_index + 1))))|5;	
	}
	else
		pte_vid[0] = (PAGE_KB+VID_MEMORY)|5;
	
	
	asm volatile("mov %0, %%eax;"
		"mov %%eax, %%cr3;"
		:
		:"r"(pdt)
		:"eax", "memory"																					//TLB Flushing
	);

	
	return 1;

}


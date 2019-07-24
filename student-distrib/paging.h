
#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

#define PAGE_MB 0x00000082
#define PAGE_KB	0x00000002
#define EXEC 32
#define VID 64


uint32_t pdt[1024] __attribute__((aligned (4096)));
uint32_t pte[1024] __attribute__((aligned (4096)));	//1024 entries for both pdt and pte, aligned by 4096
uint32_t pte_vid[1024] __attribute__((aligned(4096)));

void init_paging();
void map_page_exec(uint32_t idx);
uint32_t map_vid(uint8_t** screen_start);
uint32_t clear_vid();
uint32_t map_screen_buffer(uint32_t idx);
#endif

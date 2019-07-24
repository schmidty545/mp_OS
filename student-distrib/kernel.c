/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "_idt.h"
#include "paging.h"
#include "rtc.h"
#include "syscall.h"
#include "filesys.h"
#include "keyboard.h"
#include "process.h"
#include "pit.h"
#include "scheduler.h"

#define RUN_TESTS
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))
/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */

   

   
   
void entry(unsigned long magic, unsigned long addr) {
	/*int i;
	unsigned char buf[4096];
	unsigned char name[33];
	dentry_t dent;
	inode_t* inptr;*/
	multiboot_info_t *mbi;
	/* Clear the screen. */
	clear();
	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    	printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
    	return;
	}
	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;
	/* Print out the flags. */
	printf("flags = 0x%#x\n", (unsigned)mbi->flags);
	/* Are mem_* valid? */
	if (CHECK_FLAG(mbi->flags, 0))
    	printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);
	/* Is boot_device valid? */
	if (CHECK_FLAG(mbi->flags, 1))
    	printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);
	/* Is the command line passed? */
	if (CHECK_FLAG(mbi->flags, 2))
    	printf("cmdline = %s\n", (char *)mbi->cmdline);
	if (CHECK_FLAG(mbi->flags, 3)) {
    	int mod_count = 0;
    	int i;
    	module_t* mod = (module_t*)mbi->mods_addr;
    	while (mod_count < mbi->mods_count) {
        	printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
        	printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
        	printf("First few bytes of module:\n");
        	if(mod_count==0)
				init_filesys((unsigned int)mod->mod_start);
        	for (i = 0; i < 16; i++) {
            	printf("0x%x ", *((char*)(mod->mod_start+i)));
        	}
        	printf("\n");
        	mod_count++;
        	mod++;
    	}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
    	printf("Both bits 4 and 5 are set.\n");
    	return;
	}
	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG(mbi->flags, 5)) {
    	elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
    	printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
            	(unsigned)elf_sec->num, (unsigned)elf_sec->size,
            	(unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
	}
	/* Are mmap_* valid? */
	if (CHECK_FLAG(mbi->flags, 6)) {
    	memory_map_t *mmap;
    	printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
            	(unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
    	for (mmap = (memory_map_t *)mbi->mmap_addr;
            	(unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
            	mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
        	printf("	size = 0x%x, base_addr = 0x%#x%#x\n	type = 0x%x,  length	= 0x%#x%#x\n",
                	(unsigned)mmap->size,
                	(unsigned)mmap->base_addr_high,
                	(unsigned)mmap->base_addr_low,
                	(unsigned)mmap->type,
                	(unsigned)mmap->length_high,
                	(unsigned)mmap->length_low);
	}
	/* Construct an LDT entry in the GDT */
	{
    	seg_desc_t the_ldt_desc;
    	the_ldt_desc.granularity = 0x0;
    	the_ldt_desc.opsize  	= 0x1;
    	the_ldt_desc.reserved	= 0x0;
    	the_ldt_desc.avail   	= 0x0;
    	the_ldt_desc.present 	= 0x1;
    	the_ldt_desc.dpl     	= 0x0;
    	the_ldt_desc.sys     	= 0x0;
    	the_ldt_desc.type    	= 0x2;
    	SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
    	ldt_desc_ptr = the_ldt_desc;
    	lldt(KERNEL_LDT);
	}
	/* Construct a TSS entry in the GDT */
	{
    	seg_desc_t the_tss_desc;
    	the_tss_desc.granularity   = 0x0;
    	the_tss_desc.opsize    	= 0x0;
    	the_tss_desc.reserved  	= 0x0;
    	the_tss_desc.avail     	= 0x0;
    	the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
    	the_tss_desc.present   	= 0x1;
    	the_tss_desc.dpl       	= 0x0;
    	the_tss_desc.sys       	= 0x0;
    	the_tss_desc.type      	= 0x9;
    	the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;
    	SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);
    	tss_desc_ptr = the_tss_desc;
    	tss.ldt_segment_selector = KERNEL_LDT;
    	tss.ss0 = KERNEL_DS;
    	tss.esp0 = 0x800000;
    	ltr(KERNEL_TSS);
	}
	/* Init the PIC */
	i8259_init();
	initialize_idt();		//Initialize PIC and IDT
	keyboard_init();
	enable_irq(0);
	process_init();
	enable_irq(1);
	enable_irq(2);						//Enable IRQ for RTC, keyboard and slave
	enable_irq(8);
	init_paging();                      //initialize and activate paging
	init_pit(100);						//Initially, set frequency of PIT to 100
	init_scheduler();
	
	
	/* Initialize devices, memory, filesystem, enable device interrupts on the
 	* PIC, any other initialization stuff... */
	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
 	* IDT correctly otherwise QEMU will triple fault and simple close
 	* without showing you any output */
	
	
	
	printf("Enabling Interrupts\n");
	sti();
	
	/*name[NAMELEN]=0;
	
	int dir_fd=open((uint8_t*)".");

	while(read(dir_fd, buf, 0)==0)										//list all the files in the directory. 
		printf("%s\n", buf);*/

	
	
	/*unsigned char testfile[]="syserr";
	unsigned char buff[50000];
	if(read_dentry_by_name(testfile, &dent) !=-1)						
	{																	
		int32_t temp = open(testfile);									//reading files(by name)
		inptr=(inode_t*)FILEMEM;
		inptr+=1+dent.inode;
		read(temp, buff, inptr->length);
		for(i=0;i<inptr->length;i++)
		{
			printf("%c", buff[i]);
		}
	}
	else
		printf("Invalid File Name!\n");*/
	
	
	
	/*
	unsigned char buff[50000];
	unsigned char testname[33]={0,};
	if(read_dentry_by_index(6, &dent) !=-1)						
	{						
		for(i=0;i<NAMELEN;i++)
		{
			testname[i]=dent.name[i];
		}
		int32_t temp = open(testname);									//reading files(by index)
		inptr=(inode_t*)FILEMEM;
		inptr+=1+dent.inode;
		read(temp, buff, inptr->length);
		for(i=0;i<inptr->length;i++)
		{
			printf("%c", buff[i]);
		}
	}
	else
		printf("Invalid Index!\n");
	*/
	
	
	//int fd=open("rtc");
	//uint32_t rtc_freq=4;
	//int counter = 0;
	//write(fd, &rtc_freq, 4);
	
	/*while(1){															//testing rtc_open, read and write
		read(fd, 0, 0);				//rtc read test
		counter++;
		printf("%d", 1);
		
		//rtc_freq=2+((counter%2)*6);
		write(fd, &rtc_freq, 4);
	}*/
	while(1)
		execute((const uint8_t*)"shell");
#ifdef RUN_TESTS
	/* Run tests */
	launch_tests();
	
#endif
	/* Execute the first program ("shell") ... */
	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile (".1: hlt; jmp .1;");
}


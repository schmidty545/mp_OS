#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "types.h"
#define PASS 1
#define FAIL 0
#define VIDEO 0xb8000

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}
/* IDT Test - Example
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Check for divide by zero error
 * Files: x86_desc.h/S
 */
/*int zero_test(){
	TEST_HEADER;
	int i=1/0;
	
	return i;
}*/
/* IDT Test - Example
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: Checks for paging fault
 * Files: x86_desc.h/S
 */
int paging_test(){
	int* ptr=(int*)VIDEO;
	int i;
	
	printf("accessing video memory : %x\n", ptr);			//Shifts the video memory
	for(i=0;i<3;i++)
		*(ptr+i)=100;
	
	printf("accessing null\n");
	ptr = (int*)NULL;										//Explicitly dereference the NULL pointer
	*ptr=40;
	return 0;
}
// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("zero_test\n", zero_test());
	//TEST_OUTPUT("paging_test\n", paging_test());
	//int* ptr=0x007ffff0;
	//*ptr=40;
	//printf("%d", *ptr);
	// launch your tests here
}

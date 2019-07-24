#include "rtc.h"
#include "scheduler.h"
//#include "filesys.h"
#define MAXIMUM_RATE 15
#define MAX_FREQ 1024
#define B_REG 0x8B
#define OUT_PORT 0x70
#define IN_PORT 0x71

/*
 rtc_init
 INPUT: none
 OUTPUT: none
 SIDE EFFECTS: none
 Initializes the registers necessary for initializing the RTC in IRQ8
*/

void rtc_init() {
/* Code obtained from https://wiki.osdev.org/RTC */
 flag_rtc = 0;

 outb(B_REG, OUT_PORT);			   //Register B and disable any non maskable interrupts
 char prev = inb(IN_PORT);
 outb(B_REG, OUT_PORT);        	   //Initialization of the registers
 outb(prev | 0x40, IN_PORT);       //Obtained from OSDEV.org
}

/*
 open
 INPUT: none
 OUTPUT: 0
 SIDE EFFECT: none
 Simply sets the RTC to its default frequency of 2HZ
*/
int32_t rtc_open(uint32_t fd, uint32_t inode){
	
 rate = MAXIMUM_RATE;        //Default frequency
 
 rtc_init();				 //Call the initialization function

 process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&rtc_table;
 process_array[schedule_index].cur_fd_ptr[fd].inode = 0;
 process_array[schedule_index].cur_fd_ptr[fd].file_position = 0;
 process_array[schedule_index].cur_fd_ptr[fd].flags = 1;										//Initialize descriptor table
 
 return 0;
}
/*
 close
 INPUT: none
 OUTPUT: 0
 SIDE EFFECT: none
 Simply sets the RTC to its default frequency of 2HZ
*/

int32_t rtc_close(int32_t fd){
 //rate = MAXIMUM_RATE;        //Default frequency
 
 if(process_array[schedule_index].cur_fd_ptr[fd].flags==0){
  return -1;
 }
 
 process_array[schedule_index].cur_fd_ptr[fd].table_pointer = 0;
 process_array[schedule_index].cur_fd_ptr[fd].inode = 0;
 process_array[schedule_index].cur_fd_ptr[fd].file_position = 0;
 process_array[schedule_index].cur_fd_ptr[fd].flags = 0;										//Clear descriptor table
 
 return 0;
}

/*
 read
 INPUT: none
 OUTPUT: none
 SIDE EFFECT: none
 Only returns 0, upon receiving an interrupt
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	
	flag_rtc = 0;          //Reset the flag
 
	while(!flag_rtc){
							//Literally, do nothing until an interrupt has occurred
	}
	
	return 0;
}

/*
 write
 INPUT: 4 byte argument
 OUTPUT: 0 upon success, -1 on failure
 SIDE EFFECT: none
 This function sets the frequency of the RTC according to the input,
 a 4 byte argument containing the frequency to be set to.
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){      //Frequency as input
	if(nbytes != 4){
		return -1;
	}
 
	int32_t frequency = *((int32_t*)buf);
	int power_value;
	int is_power_of_two;      //Variables to check whether a number is power of two, and its actual power
 
 
	is_power_of_two = 0;
	power_value = 0;       
 
	while((frequency % 2 == 0) && frequency > 1){    //Algorithm for checking if number
		
		frequency /= 2;        //keep dividing the number by two
		power_value++;       //http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
		
		if(frequency == 1){
			is_power_of_two = 1;    //If quotient is 1, then it must be a power of two
		}
	}

	if(!is_power_of_two || frequency > MAX_FREQ || frequency == 0){     //If frequency is not a power of two, or greater than 1024 HZ, just set rate to 0
		rate = 0;           //For test reasons
		return -1;
	}
 
	else{
		rate = (MAXIMUM_RATE - power_value) + 1;   //Else return rate + 1, this is because freq. = 32768 >> (rate - 1), accounting for that - 1.
	}
	return 0;            //return 0 upon success
}


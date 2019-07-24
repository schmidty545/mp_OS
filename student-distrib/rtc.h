#ifndef _RTC_H
#define _RTC_H
#include "lib.h"
#include "i8259.h"
#include "filesys.h"
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
volatile char rate;  //Global variable to set rate of the RTC

fileop_table_t rtc_table;

/* Initialization of the RTC registers*/

void rtc_init();

/* Return 0 upon receiving an interrupt*/

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
/* Write a new frequency to the RTC*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
/* Just set the RTC to a default frequency*/
int32_t rtc_open(uint32_t fd, uint32_t inode);
/* Just set the RTC to a default frequency*/
int32_t rtc_close(int32_t fd);
#endif


#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "types.h"

int32_t ret_status;

int8_t arg_retrieved[1024];
/* system call read function*/
int32_t read(int32_t fd, void* buf, int32_t nbytes);
/* system call write function*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
/* system call open function*/
int32_t open(const uint8_t* filename);
/* system call close function*/
int32_t close(int32_t fd);
/* system call halt function*/
int32_t halt(uint8_t status);
/* system call execute function*/
int32_t execute(const uint8_t* command);
/* system call getargs function*/
int32_t getargs(uint8_t* buf, int32_t nbytes);
/* system call vidmap function*/
int32_t vidmap(uint8_t** screen_start);
/* system call set_handler function*/
int32_t set_handler(int32_t signum, void* handler_address);
/* system call sigreturn function*/
int32_t sigreturn(void);
/* system call to halt exception function */
int32_t halt_exception(void);
#endif

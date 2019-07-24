#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include "types.h"
#include "lib.h"
#include "filesys.h"
#define KEYBOARD_IRQ        0x01
#define KEYBOARD_PORT        0x60
#define LEFT_SHIFT_PRESS    0x2A
#define RIGHT_SHIFT_PRESS    0x36
#define LEFT_SHIFT_RELEASE    0xAA
#define RIGHT_SHIFT_RELEASE    0xB6
#define CTRL_PRESS            0x1D
#define CTRL_RELEASE        0x9D
#define L_PRESS                0x26
#define DELETE_PRESS        0x0E
#define CAPS_LOCK_PRESS        0x3A
#define LEFT_ALT_PRESS			0x38
#define LEFT_ALT_RELEASE			0xB8
#define F1_PRESS		0x3B
#define F2_PRESS		0x3C
#define F3_PRESS		0x3D
#define KEY_PRESS_BOUNDARY  0x80

#define BUFFER_SIZE 128

char terminal1_buffer[BUFFER_SIZE];
char terminal2_buffer[BUFFER_SIZE];
char terminal3_buffer[BUFFER_SIZE];
//char keyboard_buf[BUFFER_SIZE];

int counter1;
int counter2;
int counter3;
//function to set up initial conditions of the keyboard
void keyboard_init();
//function to get 
unsigned char getchar();
//function to set up initial conditions of the keyboard
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes);
//function to set up initial conditions of the keyboard
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes);
//function to set up initial conditions of the keyboard
int32_t keyboard_open(uint32_t fd, uint32_t inode);
//function to set up initial conditions of the keyboard
int32_t keyboard_close(int32_t fd);
fileop_table_t keyboard_table;
#endif


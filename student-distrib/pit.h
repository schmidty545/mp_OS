#ifndef PIT_H
#define PIT_H

#include "lib.h"

#define CHANNEL_0 0x40
#define CHANNEL_1 0x41
#define CHANNEL_2 0x42
#define MODE_COMMAND 0x43

/* global variables for testing purposes*/
int num_ticks;
int arr_index;
/* Initialization of the pic function*/
void init_pit(int frequency);

#endif




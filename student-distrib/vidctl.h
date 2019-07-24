#ifndef _VIDCTL_H_
#define _VIDCTL_H_

#include "types.h"
#include "lib.h"
/* global variable to keep track of which screen to display video memory*/
uint32_t visible_screen;

/* generic functions for initialization and switch screen*/
void init_vid();
uint32_t copy_vid(int32_t target_screen);
uint32_t switch_vid(int32_t target_screen);

#endif

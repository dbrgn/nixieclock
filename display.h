/*
    display.h
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>, released under Public Domain
*/
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "clock.h"

void display_init(void);
void display_update_time(void);
void display_hv_off(void);
void display_hv_on(void);

#endif

/*
    clock.h
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>
*/
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <inttypes.h>

#define T1_MATCHVAL (F_CPU/1000-1)

struct wallclock_t {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint16_t msec;
};

void clock_init(void);
uint16_t clock_get_rmsec();
void clock_get_wallclock(struct wallclock_t *wallclock);
void clock_set_wallclock(const struct wallclock_t *wallclock);
uint8_t get_days_in_month(uint8_t month, uint16_t year);
uint8_t clock_is_synced();

#endif

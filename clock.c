/*
    clock.c
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>, released under Public Domain
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>

#include "clock.h"
#include "display.h"

#define CLOCK_MAX_NOSYNCMIN    5        /* number of minutes that we will go without
                                       external clock before declaring unsynced */

static volatile struct wallclock_t g_wallclock;
static volatile uint16_t g_rmsec;        /* rolling millisecond count (overflows after 65535) */
static volatile uint8_t g_issynced;        /* clock is in sync with external time source */
static volatile uint8_t g_lastsyncmin;    /* last sync minute count */

/* This interrupt handler gets called when counter 1 output compare A matches */
ISR(TIMER1_COMPA_vect) {
    /* increment rolling millisecond count; let it overflow */
    g_rmsec++;

    /* increment wallclock time */
    g_wallclock.msec++;
    if (g_wallclock.msec == 1000) {
        g_wallclock.msec = 0;
        g_wallclock.sec++;

        if (g_wallclock.sec == 60) {
            g_wallclock.sec = 0;
            g_wallclock.min++;

            if (g_issynced && ((60 + g_wallclock.min - g_lastsyncmin) % 60 > CLOCK_MAX_NOSYNCMIN)) {
                /* too many minutes without sync */
                g_issynced = 0;
            }

            if (g_wallclock.min == 60) {
                g_wallclock.min = 0;
                g_wallclock.hour++;

                if (g_wallclock.hour == 24) {
                    g_wallclock.hour = 0;
                    g_wallclock.day++;

                    if (g_wallclock.day > 28 && g_wallclock.day >
                            get_days_in_month(g_wallclock.month, g_wallclock.year)) {
                        g_wallclock.day = 1;
                        g_wallclock.month++;

                        if (g_wallclock.month == 13) {
                            g_wallclock.month = 1;
                            g_wallclock.year++;
                        }
                    }
                }
            }
        }
    }

    /* update display twice per second (for blinking decimal point) */
    if (g_wallclock.msec == 0 || g_wallclock.msec == 500) {
        display_update_time();
    }
}

uint16_t clock_get_rmsec() {
    uint16_t my_rmsec;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        my_rmsec = g_rmsec;
    }
    return my_rmsec;
}

void clock_get_wallclock(struct wallclock_t *wallclock) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        memcpy(wallclock, (const void*)&g_wallclock, sizeof(struct wallclock_t));
    }
}

void clock_set_wallclock(const struct wallclock_t *wallclock) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        memcpy((void*)&g_wallclock, wallclock, sizeof(struct wallclock_t));
        g_issynced = 1;
        g_lastsyncmin = g_wallclock.min;
    }
}

void clock_init(void) {
    /* initialize wall clock to 1.1.1970 00:00:00.0 */
    g_wallclock.year = 1970;
    g_wallclock.month = 1;
    g_wallclock.day = 1;
    g_wallclock.hour = 0;
    g_wallclock.min = 0;
    g_wallclock.sec = 0;
    g_wallclock.msec = 0;

    g_rmsec = 0;
    g_issynced = 0;
    g_lastsyncmin = 0;

    /* set Timer1 to overflow once every millisecond */
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);
    OCR1A = T1_MATCHVAL;

    /* enable interrupt on match */
    TIMSK |= _BV(OCIE1A);
}

uint8_t get_days_in_month(uint8_t month, uint16_t year) {
    if (month == 2) {
        /* February = leap year calculation necessary */
        if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
            return 29;
        else
            return 28;
    } else {
        uint16_t monmask = 0x15AA;    /* 1010110101010 */
        return ((monmask >> month) & 1) ? 31 : 30;
    }
}

uint8_t clock_is_synced() {
    return g_issynced;
}

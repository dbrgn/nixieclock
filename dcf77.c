/*
    dcf77.c
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "dcf77.h"
#include "clock.h"

/* Decoder for DCF77, connected as follows:

    PD2 (INT0)        DCF input
    PD6                DCF bit received LED
*/

/*#define DCF_DEBUG*/

/* These two defines allow us to easily switch between inverted and
 * not-inverted output. */
#define DCFHI _BV(PIND2)
#define DCFLO 0

/* if the last bit was received more than 1.5 seconds ago, then this
 * signals the start of a minute. */
#define BORDER_MINSTART 1500
#define BORDERU_1BIT 240
#define BORDERL_1BIT 160
#define BORDERU_0BIT 140
#define BORDERL_0BIT 60

#define BITDINDEX(x) ((x & 0xF8) >> 3)
#define getbit(x) (bitdata[BITDINDEX(x)] & _BV(x & 0x07))
#define getbitval(x) ((getbit(x) == 0) ? 0 : 1)

static uint8_t bitdata[8];
static uint8_t bitsrecvd;
static uint8_t lastval;
static uint16_t lastmsec; /* msec clock value at last change */

uint8_t sumbits(uint8_t start, uint8_t end) {
    uint8_t mult = 1;
    uint8_t res = 0;
    while (start <= end) {
        res += getbitval(start) * mult;
        if (mult == 8) {
            mult = 10;
        } else {
            mult <<= 1; /*  * 2 */
        }
        start++;
    }
    return res;
}

uint8_t paritybits(uint8_t start, uint8_t end) {
    uint8_t res = 0;
    while (start <= end) {
        if (getbit(start)) {
            res = !res;
        }
        start++;
    }
    return res;
}

ISR(INT0_vect) {
    uint8_t val = PIND & _BV(PIND2);
    uint16_t curmsec;
    uint16_t msecdiff;

    if (val == lastval) {
        /* DCF level didn't change! */
#ifdef DCF_DEBUG
        puts("INT0 but no DCF level change");
#endif
        return;
    }
    lastval = val;

    curmsec = clock_get_rmsec();

    msecdiff = curmsec - lastmsec;
    lastmsec = curmsec;

    if (val == DCFHI) { /* Signal changed to high */
        if (msecdiff > BORDER_MINSTART) { /* Start of a new minute */
            if (bitsrecvd == 59) {
                /* Sanity checks */
                if (getbit(0)) /* start of minute is always 0 */
                    bitsrecvd++;

                if (getbit(17) == getbit(18))    /* time zone bits can't have same value */
                    bitsrecvd++;

                if (!getbit(20))    /* Start of encoded time is always 1 */
                    bitsrecvd++;

                /* Parity checks */
                if (paritybits(21, 28) != 0)    /* Minutes */
                    bitsrecvd++;
                if (paritybits(29, 35) != 0)    /* Hours */
                    bitsrecvd++;
                if (paritybits(36, 58) != 0)    /* Date */
                    bitsrecvd++;

                if (bitsrecvd == 59) {
                    /* Decode data now */
                    uint8_t min   = sumbits(21, 27);
                    uint8_t hour  = sumbits(29, 34);
                    uint8_t day   = sumbits(36, 41);
                    uint8_t month = sumbits(45, 49);
                    uint8_t year  = sumbits(50, 57);

                    if (min <= 59 && hour <= 23 && month <= 12 && year <= 99) {
                        struct wallclock_t    dcfwclock;

#ifdef DCF_DEBUG
                        /* sanity check 2 OK */
                        printf("*** DCF valid: %02d.%02d.%02d %02d:%02d", day, month, year, hour, min);
#endif

                        /* set wall clock - by the time we get here we'll probably already be
                           a few milliseconds past that DCF clock edge, but never mind, our
                           only aim is to be accurate to one second */
                        dcfwclock.year = year + 2000;    /* not Y2K1 compliant, hehe :) */
                        dcfwclock.month = month;
                        dcfwclock.day = day;
                        dcfwclock.hour = hour;
                        dcfwclock.min = min;
                        dcfwclock.sec = 0;
                        dcfwclock.msec = 0;
                        clock_set_wallclock(&dcfwclock);
                    }
                }
            }

#ifdef DCF_DEBUG
            puts("");
#endif
            bitsrecvd = 0;
        }
    } else { /* Signal changed to low */
        PORTD ^= _BV(PD6);

        if (bitsrecvd > 60) {
            return;
        }
        if ((msecdiff > BORDERL_1BIT) && (msecdiff < BORDERU_1BIT)) { /* valid '1' bit */
            bitdata[BITDINDEX(bitsrecvd)] |= _BV(bitsrecvd & 0x07);
            bitsrecvd++;

#ifdef DCF_DEBUG
            putchar('1');
#endif
        } else if ((msecdiff > BORDERL_0BIT) && (msecdiff < BORDERU_0BIT)) { /* valid '0' bit */
            bitdata[BITDINDEX(bitsrecvd)] &= (uint8_t)~_BV(bitsrecvd & 0x07);
            bitsrecvd++;

#ifdef DCF_DEBUG
            putchar('0');
#endif
        } else {
#ifdef DCF_DEBUG
            putchar('X');
#endif
        }
    }
}

void dcf77_init(void) {
    /* set INT0 for input with active pull-up */
    DDRD &= ~_BV(PD2);
    PORTD |= _BV(PD2);

    /* set LED pin for output */
    DDRD |= _BV(PD6);
    PORTD &= ~_BV(PD6);

    /* configure INT0 for interrupt on any level change */
    MCUCR &= ~_BV(ISC01);
    MCUCR |= _BV(ISC00);
}

void dcf77_enable(void) {
    GICR |= _BV(INT0);
}

void dcf77_disable(void) {
    GICR &= ~_BV(INT0);
}

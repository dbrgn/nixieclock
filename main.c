/*
    dcfreceiver.c
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>
*/
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>

#include "clock.h"
#include "dcf77.h"
#include "uart.h"
#include "display.h"

#define BAUD 9600
#include <util/setbaud.h>

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

int main(void) {
    //struct wallclock_t myclk;

    display_init();

    /* set up serial port */
    uart_init(UBRR_VALUE);

    /* set up clock */
    clock_init();

    /* set up and enable DCF77 decoder */
    dcf77_init();
    dcf77_enable();

    /* say hello */
    stdout = &mystdout;
    puts("dcfreceiver starting");

    /* enable interrupts, and we're ready to go */
    sei();

    /* idle mode - we work with interrupts */
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1) {
        sleep_mode();
    }

    /* DEBUG output */
    /*while (1) {
        clock_get_wallclock(&myclk);
        printf("\r%02d.%02d.%04d %02d:%02d:%02d.%03d %c",
            myclk.day, myclk.month, myclk.year,
            myclk.hour, myclk.min, myclk.sec, myclk.msec,
            clock_is_synced() ? '*' : ' ');

        _delay_ms(100);
    }*/

    return 0;
}


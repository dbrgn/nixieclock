/*
    display.c
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>, released under Public Domain
*/
#include <avr/io.h>
#include <util/delay.h>

#include "display.h"
#include "dcf77.h"

/* Displays the time on a set of Nixie tubes connected
   via BCD decoders (74141) as follows:


    PA3..0        H
    PA7..4        H
    PB3..0        M
    PB7..4        M
    PC3..0        S
    PC7..4        S

    PD7            HV power supply shutdown
    PD5            RH decimal point (blinks if clock synced)

    Since the emitted RFI from the switching HV power supply interferes
    with the DCF77 receiver, the HV power supply is kept off at startup
    until DCF77 sync is obtained. Then, every morning at 04:00, if the
    clock is not synced, the power supply is turned off again until a
    sync is obtained.
*/

#define HV_OFF_HOUR    4
#define HV_OFF_MINUTE 0

/* More digit mapping to correct wrong pin mapping in EAGLE... */
static uint8_t tube_digit_map[] = {1,0,9,8,7,6,5,4,3,2};

void display_init(void) {
    /* enable our ports for output */
    DDRA = 0xFF;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD |= _BV(PD5) | _BV(PD7);

    /* debugging danilo */
    PORTA = 3 | (3 << 4);
    PORTB = 3 | (3 << 4);
    PORTC = 3 | (3 << 4);
    while (1) { /* busy loop */ }

    /* digit test */
    display_hv_on();
    for (uint8_t i = 0; i < 10; i++) {
        PORTA = tube_digit_map[i] | (tube_digit_map[i] << 4);
        PORTB = tube_digit_map[i] | (tube_digit_map[i] << 4);
        PORTC = tube_digit_map[i] | (tube_digit_map[i] << 4);

        _delay_ms(250);
    }

    /* turn off HV power supply until we have DCF77 sync */
    display_hv_off();
}

void display_update_time(void) {
    struct wallclock_t time;
    clock_get_wallclock(&time);

    PORTA = tube_digit_map[(time.hour / 10)] | (tube_digit_map[time.hour % 10] << 4);
    PORTB = tube_digit_map[(time.min / 10)]  | (tube_digit_map[time.min % 10] << 4);
    PORTC = tube_digit_map[(time.sec / 10)]  | (tube_digit_map[time.sec % 10] << 4);

    /* show blinking 1 Hz LED/decimal point */
    if (time.msec < 500)
        PORTD |= _BV(PD5);
    else
        PORTD &= ~_BV(PD5);

    /* turn on HV power supply and turn off DCF77 receiver if synced */
    if (clock_is_synced()) {
        display_hv_on();
        dcf77_disable();    /* disable receiver to reduce occurrence of garbled messages */
    } else if (time.hour == HV_OFF_HOUR && time.min == HV_OFF_MINUTE && time.sec == 0) {
        /* once a day, if we don't have DCF77 sync, turn off the
           HV power supply until we do */
        display_hv_off();
        dcf77_enable();
    }
}

void display_hv_off(void) {
    PORTD |= _BV(PD7);
}

void display_hv_on(void) {
    PORTD &= ~_BV(PD7);
}

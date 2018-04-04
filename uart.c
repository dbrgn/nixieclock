/*
    uart.c
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>, released under Public Domain
*/
#include <avr/io.h>
#include <stdio.h>

#include "uart.h"

void uart_init(uint16_t ubrr) {
    /* Set baud rate */
    UBRRH = (uint8_t)(ubrr>>8);
    UBRRL = (uint8_t)ubrr;

    /* Enable receiver and transmitter */
    UCSRB = _BV(RXEN) | _BV(TXEN);

    /* Set frame format: 8 data, 1 stop bit */
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
}

int uart_putchar(char c, FILE *stream) {
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
    return 0;
}

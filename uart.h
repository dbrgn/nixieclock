/*
    uart.h
    DCF77 controlled Nixie clock project (https://neon1.net/nixieclock)
    written by Manuel Kasper <mk@neon1.net>, released under Public Domain
*/
#ifndef _UART_H_
#define _UART_H_

void uart_init(uint16_t ubrr);
int uart_putchar(char c, FILE *stream);

#endif

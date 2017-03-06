/*
 * Positionsmodul.c
 *
 * Created: 3/6/2017 2:42:26 PM
 *  Author: Yannic
 */ 


#include <avr/io.h>
#include "Positionsmodul.h"
#include <util/delay.h>
#include <avr/interrupt.h>

void uart_init(void) {
	UBRR0H = (BAUDRATE >> 8);
	UBRR0L = BAUDRATE;
	
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UCSZ01) | ( 1<< UCSZ00);
	
	UCSR0B |= (1 << RXCIE0);
	UCSR0A |= (1 << RXC0);
	
	DDRD &= ~_BV(DDD0);
	DDRD |= _BV(DDD1);
	
	sei();
}

void uart_transmit(unsigned char c) {
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

char uart_read() {
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

I

int main(void)
{
	DDRD |= (1 << LED_GREEN);
	
	uart_init();
	
	
    while(1)
    {
		uart_transmit('a');
		PORTD ^= (1 << LED_GREEN);
		_delay_ms(1000);
    }
	
	return 0;
}
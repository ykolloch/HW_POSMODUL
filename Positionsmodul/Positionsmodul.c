/*
 * Positionsmodul.c
 *
 * Created: 3/6/2017 2:42:26 PM
 *  Author: Yannic
 */ 

#define F_CPU 20000000UL

#include <avr/io.h>
#include "Positionsmodul.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

unsigned char atCom1[] = {"at+wm=3\n\r"};
unsigned char atCom2[] = {"at+p2psetdev=0,81,11,11,2388,EU\n\r"};
unsigned char atCom3[] = {"at+p2psetwps=Positionsmodul,0006,0001,11223344556677881122334455667788\n\r"};
unsigned char atCom4[] = {"AT+P2PFIND=20000,2\n\r"};
	
volatile char macAddress[18];	

volatile char REC;
volatile char recMsg[100];
volatile int msgInt = 0;

void uart_init(void) {
	UBRR0H = (BAUDRATE >> 8);
	UBRR0L = BAUDRATE;
	
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UCSZ01) | ( 1<< UCSZ00);
	
	UCSR0B |= (1 << RXCIE0);
	UCSR0A |= (1 << RXC0);
	
	sei();
}

void uart_init2(void) {
	UBRR1H = (BAUDRATE >> 8);
	UBRR1L = BAUDRATE;
	
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
	UCSR1C |= (1 << UCSZ11) | ( 1<< UCSZ10);
	
}

void uart_transmit(char c) {
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

char uart_read() {
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void uart_transmit2(char c) {
	while(!(UCSR1A & (1 << UDRE1)));
	UDR1 = c;
}

char uart_read2() {
	while(!(UCSR1A & (1 << RXC1)));
	return UDR1;
}

void uart_sendString(char temp[]) {
	for(int i=0; i < strlen(temp); i++) {
		uart_transmit(temp[i]);
	}
}

void uart_sendString2(char temp[]) {
	for(int i=0; i < strlen(temp); i++) {
		uart_transmit2(temp[i]);
	}
}

void wifiDirect_connection() {
	PORTD ^= (1 << LED_YELLOW);
	_delay_ms(1000);
	uart_sendString(atCom1);
	_delay_ms(10000);
	uart_sendString(atCom2);
	_delay_ms(500);
	uart_sendString(atCom3);
	_delay_ms(500);
	uart_sendString(atCom4);
	PORTD ^= (1 << LED_YELLOW);
}

ISR(USART0_RX_vect) {
	REC = UDR0;
	recMsg[msgInt] = REC;
	if(REC == '\n') {
		recMsg[msgInt++] = '\0';
		msgInt = 0;
		uart_sendString2(recMsg);
		memset(&recMsg[0], 0, sizeof(recMsg));
	} else {
		msgInt++;
	}
}

int main(void)
{
	DDRD |= (1 << LED_GREEN);
	DDRD |= (1 << LED_YELLOW);
	DDRD |= (1 << LED_RED);
	
	PORTD &= ~(1 << LED_GREEN);
	PORTD &= ~(1 << LED_YELLOW);
	PORTD &= ~(1 << LED_RED);
	
	uart_init();
	uart_init2();
	
	wifiDirect_connection();
	
    while(1)
    {
		PORTD ^= (1 << LED_GREEN);
		_delay_ms(500);
    }
	
	return 0;
}
/*
 * Positionsmodul.c
 *
 * Created: 3/6/2017 2:42:26 PM
 *  Author: Yannic
 */ 

#define F_CPU 20000000UL
#define ESC 27

#include <avr/io.h>
#include "Positionsmodul.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

/************************************************************************/
/* AT+Commands for Wi-Fi Direct connection.								*/
/************************************************************************/
unsigned char atCom1[] = {"at+wrxactive=1\n\r"};															//set wrxactive
unsigned char atCom2[] = {"at+wm=3\n\r"};																	//set wm=3 (Wi-Fi Direct)
unsigned char atCom3[] = {"at+p2psetdev=0,81,11,11,2388,EU\n\r"};											//set p2p device
unsigned char atCom4[] = {"at+p2psetwps=Positionsmodul,0006,0001,11223344556677881122334455667788\n\r"};	//set wps
unsigned char atCom5[] = {"AT+P2PFIND=5000,2\n\r"};															//find Wi-Fi Direct devices, 5sec.

//volatile char macAddress[] = {"7a:f8:82:cb:a3:05"};
volatile char macAddress[19];																				//MAC-Address of found Wi-Fi Direct device.
volatile char host_ip[] = {"192.168.49.1"};																	//change with get_hostIP()
volatile int start_transmission = 0;																		//Start data transmission.

/************************************************************************/
/* var for Interrupts													*/
/************************************************************************/
volatile char REC;
volatile char REC2;
volatile char recMsg[100];
volatile char recMsg2[200];
volatile int msgInt = 0;
volatile int msgInt2 = 0;

/************************************************************************/
/* var for Data transmissions											*/
/************************************************************************/
const volatile char s[] = {0x1B, 0x53, 0x30};																//Hex = <ESC> S <CID>
const volatile char p3[] = {0x1B, 0x45};																	//HEY = <ESC> E
volatile char lul[1];

/************************************************************************/
/* var for Timer														*/
/************************************************************************/
volatile int tenMilsec = 0;
volatile char old_gnssData[200];
volatile char new_gnssData[200];
volatile int check_gnssData = 100;

/************************************************************************/
/* init UART0 with Interrupts											*/
/************************************************************************/
void uart_init(void) {
	UBRR0H = (BAUDRATE >> 8);
	UBRR0L = BAUDRATE;
	
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UCSZ01) | ( 1<< UCSZ00);
	
	UCSR0B |= (1 << RXCIE0);
	UCSR0A |= (1 << RXC0);
	
	sei();
}

/************************************************************************/
/* init UART1 with Interrupts											*/
/************************************************************************/
void uart_init2(void) {
	UBRR1H = (BAUDRATE >> 8);
	UBRR1L = BAUDRATE;
	
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
	UCSR1C |= (1 << UCSZ11) | ( 1 << UCSZ10);
	
	
	UCSR1A |= (1 << RXC1);
}

/************************************************************************/
/* transmit char for UART0												*/
/************************************************************************/
void uart_transmit(char c) {
	while(!(UCSR0A & (1 << UDRE0))) {
	}
	UDR0 = c;
}

/************************************************************************/
/* read char for UART0													*/
/************************************************************************/
char uart_read() {
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

/************************************************************************/
/* transmit char for UART1												*/
/************************************************************************/
void uart_transmit2(char c) {
	while(!(UCSR1A & (1 << UDRE1)));
	UDR1 = c;
}

/************************************************************************/
/* read char for UART1													*/
/************************************************************************/
char uart_read2() {
	while(!(UCSR1A & (1 << RXC1)));
	return UDR1;
}

/************************************************************************/
/* sends String for UART0 via uart_transmit().                          */
/************************************************************************/
void uart_sendString(char temp[]) {
	for(int i=0; i < strlen(temp); i++) {
		uart_transmit(temp[i]);
	}
}

/************************************************************************/
/* sends String for UART1 via uart_transmit2().                         */
/************************************************************************/
void uart_sendString2(char temp[]) {
	for(int i=0; i < strlen(temp); i++) {
		uart_transmit2(temp[i]);
	}
}

/************************************************************************/
/* execute the AT-Commands to establish Wi-Fi Direct connection.        */
/************************************************************************/
void wifiDirect_connection() {
	PORTD ^= (1 << LED_GREEN);
	_delay_ms(1000);
	uart_sendString(atCom1);
	_delay_ms(500);
	uart_sendString(atCom2);
	_delay_ms(500);
	uart_sendString(atCom3);
	_delay_ms(500);
	uart_sendString(atCom4);
	_delay_ms(500);
	uart_sendString(atCom5);
}

/************************************************************************/
/* builds/executes AT-Commands for PPD-request an GROUPFORM with macAddress      */
/************************************************************************/
void grp_request() {
	_delay_ms(5000);
	do 
	{
		char ppd[30];
		char p1[] = {"at+p2ppd="};
		char p2[] = {",0\n\r"};
		sprintf(ppd, "%s%s%s", p1, macAddress, p2);			//add found Mac-Address
		uart_sendString(ppd);								//ppd request
		_delay_ms(5000);									//wait for safety
		
		char grp_form[45];
		char p3[] = {"at+p2pgrpform="};
		char p4[] = {",6,0,,1,0,0\n\r"};
		sprintf(grp_form, "%s%s%s", p3, macAddress, p4);	//add found Mac-Address
		uart_sendString(grp_form);							//groupform request
		_delay_ms(3000);
		return;
	} while (macAddress[0] != '\0');
}

/************************************************************************/
/* builds/executes AT+Command for TCP Connection						*/
/************************************************************************/
void tcp_connection() {
	do 
	{
		uart_sendString("at+ndhcp=1\n\r");					//needed for host_ip
		_delay_ms(500);
		char nct[27];
		char p1[] = {"at+nctcp="};
		char p2[] = {",8288\n\r"};
		sprintf(nct, "%s%s%s",p1, host_ip, p2);				//add host_ip
		uart_sendString(nct);
		_delay_ms(3000);
		//start_transmission = 1;								//start of Data Transmission
		
		PORTD ^= (1 << LED_GREEN);
		return;
	} while (host_ip[0] != '\0');
}

/************************************************************************/
/* looks for Host IP, needed for TCP Connection							*/
/************************************************************************/
void get_hostIP(char tmp[]) {
	
}

/************************************************************************/
/* looks for MAC Address, needed for PPD/GroupForm.						*/
/************************************************************************/
void get_macAddress(char temp[]) {
	char subString[10];
	char p2p_found[10] = {"p2p-dev"};		//p2p device found
	char p2p_found2[10] = {"p2v-fou"};		//backup
	strncpy(subString, &temp[0], 7);
	subString[8] = '\n';
	subString[9] = '\0';
	if(strcmp(p2p_found, subString) == 0) {
		PORTD ^= (1 << LED_RED);
		strncpy(&macAddress, &temp[14], 17);		//string copy Mac-Address
		macAddress[18] = '\0';
	} else if(strcmp(p2p_found2, subString) == 0) {
		PORTD ^= (1 << LED_RED);
		strncpy(&macAddress, &temp[10], 17);
		macAddress[18] = '\0';
	}
}

/************************************************************************/
/* Creates and Sends a String via TCP Connection.						*/
/************************************************************************/
void wifi_sendString(char data[]) {
	volatile char transTemp[200];
	const unsigned char s[] = {0x1B, 0x53, 0x30};			//Hex = <ESC> S <CID>
	//unsigned char m[] = {"Hello"};
	const unsigned char p3[] = {0x1B, 0x45};				//HEY = <ESC> E
	sprintf(transTemp, "%s%s%s", s, data, p3);
	uart_sendString(transTemp);
}

/************************************************************************/
/* Sends a single Char via TCP Connection.								*/
/************************************************************************/
void sendDataChar(char c) {
	char esc = 0x1B;
	char S = 0x53;
	char o = 0x30;
	char e = 0x45;
	uart_transmit(esc);
	uart_transmit(S);
	uart_transmit(o);
	uart_transmit(c);
	uart_transmit(esc);
	uart_transmit(e);
	uart_transmit('\n');
}

/************************************************************************/
/* INTERUPT for UART0													*/
/* Creates String from received Data for finding MAC-Address and Host IP*/
/************************************************************************/
ISR(USART0_RX_vect) {
	REC = UDR0;
	recMsg[msgInt] = REC;
	if(REC == '\n') {
		recMsg[msgInt++] = '\n';
		recMsg[msgInt++] = '\0';
		msgInt = 0;
		get_macAddress(recMsg);
		get_hostIP(recMsg);
		memset(&recMsg[0], 0, sizeof(recMsg));				//clear char array
	} else if (REC == '\r')	{
	} else {
		msgInt++;
	}
}

/************************************************************************/
/* INTERUPT for UART1													*/
/************************************************************************/
ISR(USART1_RX_vect) {
	if(start_transmission == 1) {
		recMsg2[msgInt2] = UDR1;
		if(recMsg2[msgInt2] == '\n') {
			recMsg2[msgInt2++] = '\0';
			msgInt2 = 0;
			is_gga(recMsg2);
			memset(&recMsg[0], 0, sizeof(recMsg));			//clear char array
		} else if(recMsg2[msgInt2] == '\r') {
		} else {
			msgInt2++;
		}
	}
}

void is_gga(char temp[]) {
	char subString[10];
	char gga[10] = {"GPGGA"};		//GGA message
	strncpy(subString, &temp[0], 5);
	subString[5] = '\0';
	if(strcmp(gga, subString) == 0) {
		strncpy(&new_gnssData, &temp[0], sizeof(temp));		//string copy Mac-Address
		int size = sizeof(new_gnssData);
		new_gnssData[size++] = '\0';
	}
}

/************************************************************************/
/* init for LEDs														*/
/************************************************************************/
void init_LED() {
	DDRD |= (1 << LED_GREEN);
	DDRD |= (1 << LED_RED);
	DDRD |= (1 << GNSS_RST);
	
	PORTD &= ~(1 << LED_GREEN);
	PORTD &= ~(1 << LED_RED);
	PORTD |= (1 << GNSS_RST);
}

/************************************************************************/
/* Resets the GNSS-Modul												*/
/************************************************************************/
void reset_gnss() {
	UCSR1B |= (1 << RXCIE1);
	PORTD ^= (1 << GNSS_RST);
	_delay_ms(500);
	PORTD ^= (1 << GNSS_RST);
	start_transmission = 1;
}

void init_timer2() {
	PRR0 = (0 << PRTIM2);
	
	TCCR2B = (1 << CS21);			//8bit presacale
	TCNT2 = 5;						//pre value 5-255
	
	TIMSK2 |= (1 << TOIE2);			//interrupt
	
	sei();
}


ISR(TIMER2_OVF_vect) {
	if(tenMilsec == 10) {
		check_gnssData--;
		if(check_gnssData == 0) {
			PORTD ^= (1 << LED_RED);
			if(start_transmission == 1){
				wifi_sendString(recMsg2);
			}
			check_gnssData = 100;
		}
		tenMilsec = 0;
	} else {
		tenMilsec++;		
	}
}

int main(void)
{
	init_timer2();
	init_LED();
	
	uart_init();
	uart_init2();
	
	wifiDirect_connection();
	grp_request();
	tcp_connection();
	
	reset_gnss();
	
    while(1)
    {
		/**
		if(start_transmission == 1) {
			sendDataChar();
		}	**/	
    }
	
	return 0;									//IDE avoid warning.
}
/*
 * Positionsmodul.h
 *
 * Created: 3/6/2017 2:43:28 PM
 *  Author: Yannic
 */ 


#ifndef POSITIONSMODUL_H_
#define POSITIONSMODUL_H_



#define BAUD 9600
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

#define LED_GREEN PD5
#define LED_YELLOW PD6
#define LED_RED PD7

#define TRUE 0
#define FALSE 1


#endif /* POSITIONSMODUL_H_ */
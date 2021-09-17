/*
 * softuart.h
 *
 * Software UART (bit-banging) driver, blocking i/o with timeout
 *
 * Author: Karim Bouanane
 * Hardware: ATMEGA328P
 */

#ifndef SWUART_H_
#define SWUART_H_

#include <avr/io.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>
#include "Timer.h"


#define BR_9600     // Desired baudrate

//This section chooses the correct timer values for the chosen baudrate.
#ifdef  BR_9600
	#define TICKS2COUNT         25  //!< Ticks between two bits.
	#define TICKS2WAITONE       25  //!< Wait one bit period.
	#define TICKS2WAITONE_HALF  38  //!< Wait one and a half bit period.
#endif

//Some IO, timer and interrupt specific defines.
#define ENABLE_EXTERNAL0_INTERRUPT()	(EIMSK |= _BV(INT0))
#define DISABLE_EXTERNAL0_INTERRUPT()	(EIMSK &= ~_BV(INT0))
#define ENABLE_INT0_FALLINGEDGE()		(EICRA |= _BV(ISC01))
#define CLEAR_INT0_INTERRUPT()			(EIFR |= _BV(INTF0))

#define ENABLE_TIMER_INTERRUPT()		(TIMSK0 |= _BV(OCIE0A))
#define DISABLE_TIMER_INTERRUPT()		(TIMSK0 &= ~_BV(OCIE0A))
#define CLEAR_TIMER_INTERRUPT()			(TIFR0 |= _BV(OCF0A))
#define RESET_TIMER_PRESCALAR()			(TCCR0B &= ~(_BV(CS01) | _BV(CS00)))
#define SET_TIMER_PRESCALAR()			(TCCR0B |= _BV(CS01) | _BV(CS00))

#define TX_PIN				PORTD3	// Transmit data pin, could be any digital pin
#define RX_PIN				PORTD2	// Receive data pin, must be INT0
#define SET_TX_PIN()		(PORTD |= _BV(TX_PIN))
#define CLEAR_TX_PIN()		(PORTD &= ~_BV(TX_PIN))
#define GET_RX_PIN()		(PIND & _BV(RX_PIN))

#define MAX_DELAY 0xFFFFFFFF

class SWUART
{
	
public:	// public variables

    static uint16_t bit_period;
    static uint16_t bit_half_period;
	
public:	// public methods

	// settings
    void init(uint32_t baud = 9600);
    void setBaud(uint32_t baud);
	
	// send data
    void send(char data);
    void sendString(const char *message);
    void sendString(const char *message, size_t len);
	void sendBytes(const char* bytes, size_t len);
	
	// read data
	void flush();
	bool isAvailable();
    bool read(char *data, uint32_t timeout = MAX_DELAY);
    size_t readString(char *buff, size_t len, uint32_t timeout = MAX_DELAY);
	size_t readBytes(char *bytes, size_t len, uint32_t timeout = MAX_DELAY);
	size_t readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout = MAX_DELAY);
	
	// find data
	bool find(const char *target, uint32_t timeout = MAX_DELAY);
	bool find(const char *target, size_t len, uint32_t timeout = MAX_DELAY);
	uint8_t findOneOf(const char *target1, const char *target2, uint32_t timeout = MAX_DELAY);
	uint8_t findOneOf(const char *target1, size_t len1, const char *target2, size_t len2, uint32_t timeout = MAX_DELAY);
};

#endif /* SWUART_H_ */
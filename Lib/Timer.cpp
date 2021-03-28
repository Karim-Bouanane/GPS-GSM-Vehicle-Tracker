/*
 * Timer.cpp
 *	
 *	
 *	
 *  Author: Karim Bouanane
 *	Hardware : ATMEGA328P
 */

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "Timer.h"

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#define OCR1AH_VALUE 7		// 1 ms with divider = 8
#define OCR1AL_VALUE 208   // 1 ms with divider = 8

static volatile uint32_t count_millis; // can reach up to 4,294,967,295 milliseconds which is equivalent to 497 days

void Timer::init()
{
    TCCR1B |= _BV(WGM12); // choose CTC mode

    OCR1AH = OCR1AH_VALUE; // stop when 1ms is completed
    OCR1AL = OCR1AL_VALUE;

    TIMSK1 |= _BV(OCIE1A); // enable timer1 interrupt for mode OCR

    sei();               // enable global interrupt
    TCCR1B |= _BV(CS11); // choose divider 8
}

uint32_t Timer::now()
{
    uint32_t millis;
	
	// avoid concurrent access to count_millis variable
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		millis = count_millis;
	}
	
    return millis;
}

ISR(TIMER1_COMPA_vect)
{
    ++count_millis;
}
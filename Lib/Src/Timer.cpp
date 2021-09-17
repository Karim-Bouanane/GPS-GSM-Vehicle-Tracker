/*
 * Timer.cpp
 *	
 * This library uses the Timer1 peripheral with interrupts
 * to give the time in ms since the first initialization of timer
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */


#include "Timer.h"

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#define OCR1AH_VALUE 7		// to calculate 1 ms with clock divider = 8
#define OCR1AL_VALUE 208   // 

static volatile uint32_t count_millis; // can reach up to 4,294,967,295 milliseconds which is equivalent to 497 days
static volatile bool enabled = false;

inline static void init()
{
	enabled = true;
    
	TCCR1B |= _BV(WGM12);	// choose CTC mode

    OCR1AH = OCR1AH_VALUE; 
    OCR1AL = OCR1AL_VALUE;	// trigger overflow when 1ms is completed

    TIMSK1 |= _BV(OCIE1A);	// enable timer1 interrupt for OCR mode

    sei();					// enable global interrupt
    TCCR1B |= _BV(CS11);	// choose divider 8 and start timer
}

uint32_t timerNow()
{
	if(enabled == false)
		init();

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
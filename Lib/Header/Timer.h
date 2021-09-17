/*
 * Timer.h
 *
 * This library uses the Timer1 peripheral with interrupts
 * to give the time in ms since the first initialization of timer
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

uint32_t timerNow();


#endif /* TIMER_H_ */
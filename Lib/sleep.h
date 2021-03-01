/*
 * sleep.h
 *
 * Created: 21/02/2021 13:53:36
 *  Author: Karim Bouanane
 */ 

#ifndef SLEEP_H_
#define SLEEP_H_


#include <avr/io.h>
#define _BV(x) (uint8_t)(1 << x)


/* Sleeping Mode

You can find useful information about current consumption and enabled 
modules during each sleep mode

https://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/

Note: Power-down Mode
    Only an external reset, a watchdog system reset, a watchdog interrupt,
a brown-out reset, a 2-wire serial interface address match, an external
level interrupt on INT0 or INT1, or a pin change interrupt can wake up
the MCU                                                 
*/  

typedef enum
{
	mode_idle = 0,
	mode_adc = _BV(SM0),
	mode_pwr_down = _BV(SM1),
	mode_pwr_save = (_BV(SM0) | _BV(SM1)),
	mode_standby = (_BV(SM1) | _BV(SM2)),
	mode_ext_standby = (_BV(SM0) | _BV(SM1) | _BV(SM2))
	
} sleep_mode;


#define set_sleep_mode(mode)	(SMCR |= _BV(mode))
#define sleep_enable()			(SMCR |= _BV(SE))
#define sleep_disable()			(SMCR &= ~_BV(SE))
#define sleep_cpu()				__asm__ __volatile__ ( "sleep" "\n\t" :: )


#define goTosleep(mode)	\
do {					\
	set_sleep_mode(mode);	\
	cli();					\
	sleep_enable();			\
	sei();					\
	sleep_cpu();			\
	sleep_disable();		\
	sei();					\
} while (0)


/*
	Watchdog Timer
*/

typedef enum 
{
	SLEEP_15MS,
	SLEEP_30MS,
	SLEEP_60MS,
	SLEEP_120MS,
	SLEEP_250MS,
	SLEEP_500MS,
	SLEEP_1S,
	SLEEP_2S,
	SLEEP_4S,
	SLEEP_8S = _BV(WDP3) | _BV(WDP0),
	SLEEP_FOREVER						// Use SLEEP_FOREVER to use other wake up resource
} period_t;


#define wdt_reset()					__asm__ __volatile__ ("wdr")
#define wdt_enable_interrupt()		(WDTCSR =  _BV(WDCE) | _BV(WDE))
#define wdt_set_timeout(period)		(WDTCSR =  _BV(WDIE)| period)

#define wakeupAfter(period)	\
do {						\
	cli();					\
	wdt_reset();			\
	wdt_enable_interrupt(); \
	wdt_set_timeout(period);\
	sei();					\
} while(0)


/*
	Watchdog Timer interrupt service routine. This routine is
	required to allow automatic WDIF and WDIE bit clearance in
	hardware
*/

ISR(WDT_vect)
{
	// Do job
	wdt_reset();
}


/* 
	Note: When waking up

	If an enabled interrupt occurs while the MCU is in a sleep mode, the MCU wakes up. 
	The MCU is then halted for four cycles in addition to the start-up time, executes 
	the interrupt routine, and resumes execution from the instruction following SLEEP.
	The contents of the register file and SRAM are unaltered when the device wakes up 
	from sleep. If a reset occurs during sleep mode, the MCU wakes up and executes 
	from the reset vector.

*/


#endif /* SLEEP_H_ */
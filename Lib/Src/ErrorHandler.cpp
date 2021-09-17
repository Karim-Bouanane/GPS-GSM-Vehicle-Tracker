/*
 * ErrorHandler.cpp
 *
 * This library is used to handle errors occurred by GPS and GSM/GPRS module
 * Whenever the function errorHandler is called the MCU atmega328 will turn
 * off all peripherals and lower it frequency at the lowest level
 * 
 *
 * Author: Karim Bouanane
 * Hardware: ATMEGA328P
 */ 

#include "ErrorHandler.h"

#define CLOCK_FREQ_PRESCALER	clock_div_256

const uint16_t wait = 300 ;

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}

static void init()
{
	lowPowerIO();
	DDRB |= _BV(0) | _BV(2);	// the two leds are attached to those pins
}

static void toggle(bool ledGPRS, bool ledGPS, uint8_t times)
{
	uint8_t temp_times = times;
	
	while (1)
	{
		while(times--)
		{
			PORTB |= ((ledGPS << 0) | (ledGPRS << 2));
			_delay_ms(wait);
			
			PORTB &= ~((ledGPS << 0) | (ledGPRS << 2));
			_delay_ms(wait);
		}
		
		_delay_ms( 10 * wait ); 
		times = temp_times;
	}
}

static void divideClock(uint8_t prescalar)
{
	clock_prescale_enable();
	clock_prescale_set(prescalar);
}

static void turnOffAllPeripheral()
{
	disable_adc();
	power_adc_off();
	power_ac_off();
	power_spi_off();
	power_timer0_off();
	power_timer1_off();
	power_timer2_off();
	power_twi_off();
	power_usart0_off();
}

void errorHandler(uint8_t module, uint8_t code)
{
	init();
	turnOffAllPeripheral();
	divideClock(CLOCK_FREQ_PRESCALER);	// reduce frequency to 62,5 KHz
	
	if(module == GPS_MODULE)
	{
		toggle(false, true, code);
	}
	else if (module == GPRS_MODULE)
	{
		toggle(true, false, code);
	}
	else	// BOTH MODULE
	{
		toggle(true, true, code);
	}
}
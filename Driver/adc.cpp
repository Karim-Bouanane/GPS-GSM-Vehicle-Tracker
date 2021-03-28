/*
 *	adc.cpp
 *
 *	Take measurement from analog to digital converter
 *
 *  Author: Karim Bouanane
 *
 *	Hardware : ATMEGA328P
 */

#include <avr/io.h>
#include <stdlib.h>
#include "adc.h"

#define REFERENCE_VOLTAGE 1077UL // measured from pin AREF, value in mV
#define NUM_SAMPLES 10	// number of samples to calibrate the input values
#define R1 100 // unit in Kohm but it doesn't matter in formula
#define R2 470 // unit in Kohm

void ANALOG::init()
{
    ADCSRA |= _BV(ADEN);                            // Enable ADC
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // Select divider 128
    ADMUX |= _BV(REFS1) | _BV(REFS0);               // Select internal reference voltage 1.1V
};

uint16_t ANALOG::getValueFrom(ADC_PIN pin)
{
    DDRC &= ~_BV(pin);            // ADC pin as input
    ADMUX = (ADMUX & 0xF8) | pin; // Select the input ADC pin

    uint16_t val_input = 0, sum = 0;
    uint32_t voltage_cal = 0, voltage = 0;
    uint8_t sample_count = NUM_SAMPLES;

    while (sample_count--)
    {
        ADCSRA |= _BV(ADSC); // Start conversion
        while (ADCSRA & (1 << ADSC))
            ;                                               // wait conversion to complete
        val_input = ((uint8_t)ADCL) | ((uint8_t)ADCH << 8); // get 10bits result of conversion
        sum += val_input;									// sum read values
    }

    voltage_cal = (uint32_t)(sum * REFERENCE_VOLTAGE) / (uint32_t)(1024UL * NUM_SAMPLES); // get calibrated voltage value
    voltage = (voltage_cal * (R1 + R2)) / (R1);                                         // extract the voltage of battery using voltage divider formula

    return voltage; // in millivolts
}

char* ANALOG::getStrValueFrom(ADC_PIN pin)
{	
	uint16_t val = getValueFrom(pin);	// get value in integer format
	return itoa(val,analogStrValue,10);	// convert integer and return reference 
										// to analogStrValue string
}
/*
 * ErrorHandler.h
 *
 * This library is used to handle errors occurred by GPS and GSM/GPRS module
 * Whenever the function errorHandler is called the MCU atmega328 will turn
 * off all peripherals and lower it frequency at the lowest level
 * 
 *
 * Author: Karim Bouanane
 * Hardware: ATMEGA328P
 */ 


#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#ifndef F_CPU
	#define F_CPU 62500UL
#endif

#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include "Power.h"

#define GPRS_MODULE		0
#define GPS_MODULE		1
#define BOTH_MODULE		2

void errorHandler(uint8_t module, uint8_t code);

#endif /* ERRORHANDLER_H_ */
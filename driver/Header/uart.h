/*
 * uart.h
 * 
 * UART driver, blocking i/o with timeout
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "Timer.h"

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#define MAX_DELAY 0xFFFFFFFF

class UART
{
	
public:	// public methods 
	
	// settings
	void setBaud(uint32_t baud);
	void init(uint32_t baud = 9600);
	
	// send data
	void send(char data);
	void sendString(const char *message);
	void sendBytes(const char* bytes, size_t len);
	void sendString(const char *message, size_t len);
	
	// read data
	void flush();
	bool read(char *data, uint32_t timeout = MAX_DELAY);
	size_t readString(char *buff, size_t len, uint32_t timeout = MAX_DELAY);
	size_t readBytes(char *buff, size_t len, uint32_t timeout = MAX_DELAY);
	size_t readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout = MAX_DELAY);
	
	// find data
	bool find(const char *target, uint32_t timeout = MAX_DELAY);
	bool find(const char *target, size_t len, uint32_t timeout = MAX_DELAY);
	uint8_t findOneOf(const char *target1, const char *target2, uint32_t timeout = MAX_DELAY);
	uint8_t findOneOf(const char *target1, size_t len1, const char *target2, size_t len2, uint32_t timeout = MAX_DELAY);
};

#endif /* UART_H_ */

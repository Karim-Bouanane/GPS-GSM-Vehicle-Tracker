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

#include "Timer.h"

#ifndef F_CPU
	#define F_CPU 16000000UL		// Frequency used is 16Mhz
#endif

class UART
{
private:
	Timer *uartTime;
	
public:
	UART(Timer *timer);
		
	void init();
	void init(uint32_t baud);
	void setBaud(uint32_t baud);
	void send(char);
	void sendString(const char *message);
	void sendString(const char *message, int len);
	void read(char *data);
	bool read(char *data, uint32_t timeout);
	size_t readString(char *buff, size_t len);
	size_t readString(char *buff, size_t len, uint32_t timeout);
};

#endif /* UART_H_ */

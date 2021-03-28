/*
 * uart.cpp
 *
 * UART driver, blocking i/o with timeout
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

#include "Timer.h"
#include "uart.h"

UART::UART(Timer *timer) : uartTime(timer)
{
}

void UART::setBaud(uint32_t baud)
{
    uint16_t ubrr = ((F_CPU / 16UL) / baud) - 1;

    UBRR0L = ubrr;        // Load lower 8-bits of the baud rate
    UBRR0H = (ubrr >> 8); // Load upper 8-bits
}

void UART::init()
{
    init(9600); // initialize with default baud 9600
}

void UART::init(uint32_t baud)
{
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   // Turn on transmission and reception
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // character size 8

    setBaud(baud);
}

static bool isReceiveComplete(void) { return UCSR0A & (1 << RXC0); }

static bool isTransmitComplete(void) { return UCSR0A & (1 << TXC0); }

static bool isDataEmpty(void) { return UCSR0A & (1 << UDRE0); }

void UART::send(char ch)
{
    while (!isDataEmpty())
        ;      // Wait for empty transmit buffer
		
    UDR0 = ch; // Transmit data
	
    while (!isTransmitComplete())
        ; // Wait for transmission to complete
}

void UART::sendString(const char *message, int len)
{
    while (len-- && *message != '\0')
        send(*message++);
}

void UART::sendString(const char *message)
{
    while (*message != '\0')
        send(*message++);
}

void UART::read(char *data)
{
    while (!isReceiveComplete())
        ;         // Wait till data is received
		
    *data = UDR0; // return the data read
}

bool UART::read(char *data, uint32_t timeout)
{
    uint32_t prev = uartTime->now(); // keep previous time before entering the while loop
	
    while (!isReceiveComplete())
    {
        if (uartTime->now() - prev > timeout) // be sure not exceed the timeout
            return false;
    }
	
    *data = UDR0; // return the data read
    return true;
}

size_t UART::readString(char *buff, size_t len)
{
	size_t i = len;
	
    while (len--)	// decrement until reaching 0
    {
		read(buff);	// read and store data in given array 
			
		if(*buff == 0 || *buff == '\n') // string terminator
			break;
			
		++buff;	// move to the next address
    }
	
	*buff = 0; // close array
	return i - len; // return number of read characters
}

size_t UART::readString(char *buff, size_t len, uint32_t timeout)
{
	size_t i = len;
	uint32_t prev = uartTime->now();
	
	while(len--)	// decrement until reaching 0
	{
		while (!isReceiveComplete())	// Wait till data is received 
		{
			if( (uartTime->now() - prev) > timeout ) // be sure not exceed the timeout
				return 0;
		}

		*buff = UDR0; // store data in buff
		
		if(*buff == 0 || *buff == '\n') // verify string terminator
			break;
			
		++buff;	// move to the next address
	}
	
	*buff = 0; // close array
	return i - len; // return number of read characters
}
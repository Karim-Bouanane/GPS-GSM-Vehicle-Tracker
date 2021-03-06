/*
 * uart.cpp
 *
 * Created: 08/02/2021 16:36:32
 *  Author: Karim Bouanane
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "uart.h"

#include <avr/delay.h>

#ifndef F_CPU
	#define F_CPU 16000000UL				   // Frequency used is 16Mhz
#endif

void UART::init()
{
	/* Frame Format */
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);	 // Turn on transmission and reception 
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // character size 8
	
	setBaud(9600);	// Set default baud 9600

	/*Enable Interrupts*/
	// sei();
	// UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0); /* Enable Transmit and receive
	// interrupts*/
}

void UART::init(uint32_t baud)
{
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);	 // Turn on transmission and reception
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // character size 8
	
	setBaud(baud);
}

void I2C::setBaud(uint32_t baud)
{
	uint16_t ubrr = ((F_CPU / (16UL * baud)) - 1);
	
	UBRR0L = (uint8_t)(ubrr);	   // Load lower 8-bits of the baud rate 
	UBRR0H = (uint8_t)(ubrr >> 8); // Load upper 8-bits
}

static bool isReceiveComplete(void) { return UCSR0A & (1 << RXC0); }

static bool isTransmitComplete(void) { return UCSR0A & (1 << TXC0); }

static bool isDataEmpty(void) { return UCSR0A & (1 << UDRE0); }

unsigned char UART::readChar()
{
	while (!isReceiveComplete()); // Wait till data is received 
	return UDR0;	// return the read data
}

void UART::sendChar(unsigned char ch)
{
	while (!isDataEmpty()); // Wait for empty transmit buffer
	UDR0 = ch;	// Transmit data
	while (!isTransmitComplete()); // Wait for transmission to complete
}

void UART::sendString(const char *message, int len)
{
	while (len-- && *message != '\0')	
		sendChar(*message++);			
}

void UART::sendString(const char *message)
{
	while (*message != '\0')
		sendChar(*message++);
}

void UART::readString(unsigned char *buff, int len)
{
	while (len--)
	{
		if ((*buff++ = readChar()) == '\n')
		{
			*buff = 0;
			break;
		}
	}
}

// ISR(USART_RX_vect) /* USART Rx Complete */
// {
//     // data = UDR0;
// }
//
// ISR(USART_UDRE_vect)
// { /* USART, Data Register Empty */
// }
//
// ISR(USART_TX_vect)
// { /* USART Tx Complete */
// }

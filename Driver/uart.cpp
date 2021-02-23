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

#define F_CPU 16000000UL // Frequency used is 16Mhz
#define BAUD 9600 // Desired baud rate
#define UBRR ((F_CPU / (16UL * BAUD)) - 1) // Value to load in UBRR to set the defined baud rate

void UART::init()
{
    /* Frame Format */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   /* Turn on transmission and reception */
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); /* character size 8 */
    // UCSR0C |= (1 << UPM01); /* Enable even parity*/
    // UCSR0C &= ~( 1 << USBS0); /* 1 Stop bit*/

    /* Baud rate Clock*/
    UBRR0L = (uint8_t)(UBRR);        /* Load lower 8-bits of the baud rate */
    UBRR0H = (uint8_t)(UBRR >> 8); /* Load upper 8-bits*/

    /*Enable Interrupts*/
    // sei();
    // UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0); /* Enable Transmit and receive
    // interrupts*/
}

static bool uart_isReceiveComplete(void) { return UCSR0A & (1 << RXC0); }

static bool uart_isTransmitComplete(void) { return UCSR0A & (1 << TXC0); }

static bool uart_isDataEmpty(void) { return UCSR0A & (1 << UDRE0); }

unsigned char UART::readChar()
{
    while (!uart_isReceiveComplete())
        ; /* Wait till data is received */
    return UDR0;
}

void UART::sendChar(unsigned char ch)
{
    while (!uart_isDataEmpty())
        ; /* Wait for empty transmit buffer*/

    UDR0 = ch;

    while (!uart_isTransmitComplete())
        ;
}

void UART::sendString(unsigned char *message, int len)
{
    while (len-- && *message != '\0')
        sendChar(*message++);
}

void UART::readString(unsigned char *buff, int len)
{
    while (len--)	{
		if((*buff++ = readChar()) == '\n'){
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

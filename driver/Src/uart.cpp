/*
 * uart.cpp
 *
 * UART driver, blocking i/o with timeout
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */

#include "uart.h"


// Debugging function

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}

static bool isReceiveComplete(void) { return UCSR0A & _BV(RXC0); }

static bool isTransmitComplete(void) { return UCSR0A & _BV(TXC0); }

static bool isDataEmpty(void) { return UCSR0A & _BV(UDRE0); }
	

/**** Settings ****/

void UART::setBaud(uint32_t baud)
{
    uint16_t ubrr = ((F_CPU / 16UL) / baud) - 1;

    UBRR0L = ubrr;        // load lower 8-bits of the baud rate
    UBRR0H = (ubrr >> 8); // load upper 8-bits
}

void UART::init(uint32_t baud)
{
	setBaud(baud);
	
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   // enable uart transmission and reception
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // choose size 8 bits for the character
}


/**** Send data methods ****/

void UART::send(char data)
{
    while (isDataEmpty() == false)
        ; // wait for empty transmit buffer

    UDR0 = data; // transmit data

    while (isTransmitComplete() == false)
        ; // wait for transmission to complete
}

void UART::sendString(const char *message)
{
	while (*message != '\0')
	send(*message++);
}

void UART::sendString(const char *message, size_t len)
{
    while (*message != '\0' && len--)
        send(*message++);
}

void UART::sendBytes(const char* bytes, size_t len)
{
	while(len--)
		send(*bytes++);
}


/**** Read data methods ****/

bool UART::read(char *data, uint32_t timeout)
{
    uint32_t prev = timerNow();				// save previous time before entering the while loop

    while (isReceiveComplete() == false)
    {
        if (timerNow() - prev > timeout)	// be sure not exceed the timeout
            return false;					// timeout is reached
    }
	
    *data = UDR0; // return the read data
    return true; // successful read
}

size_t UART::readString(char *buff, size_t len, uint32_t timeout)
{
	return readStringUntil('\n', buff, len, timeout);
}

size_t UART::readBytes(char *buff, size_t len, uint32_t timeout)
{
	size_t i = len;

	while (len--) // decrement until reaching 0
	{
		if (read(buff, timeout) == false)
			break;		// timeout is reached
		
		++buff; // move to the next address in buff array
	}
	
	return i - len; // return number of read characters
}

size_t UART::readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout)
{
	size_t i = len;

	while (len--) // decrement until reaching 0
	{
		if (read(buff, timeout) == false)
		return 0; // timeout is reached

		if (*buff == terminator) // verify string terminator
		{
			++buff; // include terminator in buffer
			break;
		}
		else
			++buff; // move to the next address in buff array
	}

	*buff = 0;      // close array
	return i - len; // return number of read characters
}


/**** Find data methods ****/

bool UART::find(const char *target, uint32_t timeout)
{
	return find(target, strlen(target), timeout);
}

bool UART::find(const char *target, size_t len ,uint32_t timeout)
{
	size_t temp_len = len;
	const char *temp_buff = target;
	uint32_t prev = timerNow();
	
	while(len--)
	{
		while (isReceiveComplete() == false)
		{
			if (timerNow() - prev > timeout)	// be sure not exceed the timeout
				return false;					// timeout is reached
		}
		
		if(UDR0 == *target)
		{
			target++;			// move to the next byte of target
		}
		else
		{
			// repeat comparison from the beginning
			target = temp_buff;
			len = temp_len;
		}
	}
	
	return true;
}

uint8_t UART::findOneOf(const char *target1, const char *target2 ,uint32_t timeout)
{
	return findOneOf(target1, strlen(target1), target2, strlen(target2), timeout);
}

uint8_t UART::findOneOf(const char *target1, size_t len1, const char *target2, size_t len2 ,uint32_t timeout)
{
	size_t temp_len1 = len1;
	size_t temp_len2 = len2;
	
	const char *temp_buff1 = target1;
	const char *temp_buff2 = target2;
	
	uint32_t prev = timerNow();

	while(len1 != 0 && len2 != 0)
	{
		while (isReceiveComplete() == false)
		{
			if (timerNow() - prev > timeout)	// be sure not exceed the timeout
				return false;					// timeout is reached
		}
				
		if(UDR0 == *target1)
		{
			target1++;			// move to the next byte of target
			len1--;
		}
		else
		{
			// repeat comparison from the beginning
			target1 = temp_buff1;
			len1 = temp_len1;
		}
		
		if(UDR0 == *target2)
		{
			target2++;			// move to the next byte of target
			len2--;
		}
		else
		{
			// repeat comparison from the beginning
			target2 = temp_buff2;
			len2 = temp_len2;
		}
	}
	
	return len1 == 0 ? 1: 2;
}
/*
 * sfuart.cpp
 *
 * Software UART (bit-banging) driver, blocking i/o with timeout
 * Based on the application note AVR304
 *
 * Author: Karim Bouanane
 * Hardware: ATMEGA328P
 */

#include "swuart.h"


typedef enum
{
	IDLE,                                   //!< Idle state, both transmit and receive possible.
	TRANSMIT,                               //!< Transmitting byte.
	TRANSMIT_STOP_BIT,                      //!< Transmitting stop bit.
	RECEIVE,                                //!< Receiving byte.
	DATA_PENDING                            //!< Byte received and ready to read.

}AsynchronousStates_t;


static volatile AsynchronousStates_t state;	//!< Holds the state of the UART.
static volatile unsigned char TXData;		//!< Data to be transmitted.
static volatile unsigned char TXBitCount;	//!< TX bit counter.
static volatile unsigned char RXData;		//!< Storage for received bits.
static volatile unsigned char RXBitCount;	//!< RX bit counter.
static volatile bool available;
static volatile uint8_t	TXString;


inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}


/**** Settings ****/

void SWUART::init(uint32_t baud)
{
    // pin receiver
    DDRD &= ~_BV(RX_PIN);		// RX_PIN as input
    PORTD |= _BV(RX_PIN);		// RX_PIN mode tri-stated

    // pin transmitter
    SET_TX_PIN();				// set the TX line to idle state
    DDRD |= _BV(TX_PIN);		// TX_PIN as output

	// Timer0
	DISABLE_TIMER_INTERRUPT();
	TCCR0A = 0x00;				// init
	TCCR0B = 0x00;				// init
	TCCR0A |= _BV(WGM01);		// CTC mode
	TCCR0B |=  _BV(CS01);		// prescaler is 8

	// External interrupt INT0
	EICRA = 0x00;               // init
	ENABLE_INT0_FALLINGEDGE();
	ENABLE_EXTERNAL0_INTERRUPT();
    
	// enable global interrupt
	sei(); 

	//Internal State Variable
	state = IDLE;
	available = false;
	TXString = 0;
}


ISR(INT0_vect)
{
	state = RECEIVE;				// change to receive state
	DISABLE_EXTERNAL0_INTERRUPT();	// disable external interrupt during reception
	
	DISABLE_TIMER_INTERRUPT();		// disable timer0 to change its registers
	RESET_TIMER_PRESCALAR();		// reset prescaler counter
	OCR0A = TICKS2WAITONE_HALF;		// count one and half period after the falling edge is trigged
	TCNT0 = 0;						// clear counter register
	SET_TIMER_PRESCALAR();			// start timer

	RXBitCount = 0;					// clear received bit counter

	CLEAR_TIMER_INTERRUPT();		// ensure timer0 interrupt is cleared
	ENABLE_TIMER_INTERRUPT();		// enable timer0 interrupt
}


ISR(TIMER0_COMPA_vect)
{
	switch (state) {
		
		case TRANSMIT:
		
			if( TXBitCount < 8 ) 
			{		
				if( TXData & 0x01 ) 
				{
					SET_TX_PIN();			// send a logic 1 on the TX_PIN
				}
				else 
				{                    
					CLEAR_TX_PIN();			// send a logic 0 on the TX_PIN
				}
				
				TXData = TXData >> 1;		// bitshift the TX buffer
				TXBitCount++;				// increment TX bit counter
			}
			
			// send stop bit
			else	
			{
				SET_TX_PIN();				// set the TX line to idle state
				state = TRANSMIT_STOP_BIT;
				CLEAR_INT0_INTERRUPT();
			}
		break;

		// go to idle after stop bit was sent
		case TRANSMIT_STOP_BIT:
			
			DISABLE_TIMER_INTERRUPT();		// stop the timer interrupts
			state = IDLE;					// go back to idle
			
			if(TXString == 0)
				ENABLE_EXTERNAL0_INTERRUPT();	// enable reception again
				
		break;

		// receive byte
		case RECEIVE:

			OCR0A = TICKS2WAITONE;			// count one period after the falling edge is trigged
			
			if( RXBitCount < 8 )
			{
				RXBitCount++;
				RXData = (RXData>>1);		// shift due to receiving LSB first
				
				if( GET_RX_PIN() != 0 )
				{
					RXData |= 0x80;			// if a logical 1 is read, let the data mirror this
				} 
			}

			// done receiving
			else 
			{
				available = true;
				state = IDLE;  
				DISABLE_TIMER_INTERRUPT();		// disable timer0 interrupt
				CLEAR_INT0_INTERRUPT();			// reset flag not to enter the ISR one extra time
				ENABLE_EXTERNAL0_INTERRUPT();	// enable interrupt to receive more bytes
			}
		break;

		default:
			state = IDLE;                       // error, should not occur. Going to a safe state
	}
}

/**** Send data methods ****/

void SWUART::send(char data)
{
	while( state != IDLE );
	
	DISABLE_EXTERNAL0_INTERRUPT();		// disable reception
	
	state = TRANSMIT;
	TXData = data;						// put byte into TX buffer
	TXBitCount = 0;

	RESET_TIMER_PRESCALAR();			// reset prescaler counter
	OCR0A = TICKS2COUNT;				// count one period after sending the first bit
	TCNT0 = 0;							// clear counter register
	SET_TIMER_PRESCALAR();				// start prescaler clock

	CLEAR_TX_PIN();						// clear TX line...start of preamble
	
	CLEAR_TIMER_INTERRUPT();
	ENABLE_TIMER_INTERRUPT();			// enable interrupt
}


void SWUART::sendString(const char *message)
{
	sendString(message, strlen(message));
}

void SWUART::sendString(const char *message, size_t len)
{
	TXString = len;
	
	while (len-- && *message != '\0')
	{
		send(*message++);
		TXString--;
	}
		
}

void SWUART::sendBytes(const char* bytes, size_t len)
{
	while(len--)
		send(*bytes++);
}


/**** Read data methods ****/

bool SWUART::isAvailable()
{
	if(available)
	{
		available = false;
		return true;
	}
	
	return false;
}

bool SWUART::read(char *data, uint32_t timeout)
{
	uint32_t prev = timerNow();				// save previous time before entering the while loop

	while (isAvailable() != true)
	{
		if (timerNow() - prev > timeout)	// be sure not exceed the timeout
			return false;					// timeout is reached
	}
	
	*data = RXData;
	return true;
}

size_t SWUART::readString(char *buff, size_t len, uint32_t timeout)
{
	return readStringUntil('\n', buff, len, timeout);
}

size_t SWUART::readBytes(char *buff, size_t len, uint32_t timeout)
{
	size_t i = len;

	while (len--)	// decrement until reaching 0
	{
		if (read(buff, timeout) == false)
			break;	// timeout is reached
		
		++buff;		// move to the next address in buff array
	}

	return i - len; // return number of read characters
}

size_t SWUART::readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout)
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

bool SWUART::find(const char *target, uint32_t timeout)
{
	return find(target, strlen(target), timeout);
}

bool SWUART::find(const char *target, size_t len, uint32_t timeout)
{
	size_t temp_len = len;
	const char *temp_buff = target;
	uint32_t prev = timerNow();
	
	while(len!=0)
	{
		while (isAvailable() != true)
		{
			if (timerNow() - prev > timeout)	// be sure not exceed the timeout
				return false;					// timeout is reached
		}
		
		if(RXData == *target)
		{	
			target++;			// move to the next byte of target
			len--;
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

uint8_t SWUART::findOneOf(const char *target1, const char *target2, uint32_t timeout)
{
	return findOneOf(target1, strlen(target1), target2, strlen(target2), timeout);
}
	
uint8_t SWUART::findOneOf(const char *target1, size_t len1, const char *target2, size_t len2, uint32_t timeout)
{
	size_t temp_len1 = len1;
	size_t temp_len2 = len2;
	const char *temp_buff1 = target1;
	const char *temp_buff2 = target2;
	
	uint32_t prev = timerNow();

	while(len1!=0 && len2!=0)
	{
		while (isAvailable() != true)
		{
			if (timerNow() - prev > timeout)	// be sure not exceed the timeout
				return 0;						// timeout is reached
		}

		if(RXData == *target1)
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
		
		if(RXData == *target2)
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
/*
 * softuart.h
 *
 * Created: 16/02/2021 14:28:13
 *  Author: Karim Bouanane
 */ 


#ifndef SOFTUART_H_
#define SOFTUART_H_

#include <avr/io.h>

//This section definer the correct timer values for the chosen baudrate 9600.
#define BAUD_RATE 9600

#define TICKS2COUNT			25      // Ticks between two bits.
#define TICKS2WAITONE		25      // Wait one bit period.
#define TICKS2WAITONE_HALF	38		 // Wait one and a half bit period.

#define INTERRUPT_EXEC_CYCL   9       //!< Cycles to execute interrupt rutine from interrupt.

//Some IO, timer and interrupt specific defines.
#define ENABLE_TIMER_INTERRUPT()		(TIMSK0 |= (1 << OCIE0A))
#define DISABLE_TIMER_INTERRUPT()		(TIMSK0 &= ~(1 << OCIE0A))
#define CLEAR_TIMER_INTERRUPT()			(TIFR0 |= ((1 << OCF0A)))
#define ENABLE_EXTERNAL0_INTERRUPT()	(EIMSK |= (1 << INT0))
#define DISABLE_EXTERNAL0_INTERRUPT()	(EIMSK &= ~(1 << INT0))
#define TX_PIN PD3						// Transmit data pin
#define RX_PIN PD2						// Receive data pin, must be INT0

#define TCCR TCCR0A						// Timer/Counter Control Register
#define TCCR_P TCCR0A					// Timer/Counter Control (Prescaler) Register
#define OCR OCR0A						// Output Compare Register
#define PRESCALAR_64 (( 1 << CS01 ) | (1 << CS00))

#define EXT_IFR EIFR                     // External Interrupt Flag Register
#define EXT_ICR EICRA                    // External Interrupt Control Register

#define SET_TX_PIN() (PORTD |= (1 << TX_PIN))
#define CLEAR_TX_PIN() (PORTD &= ~(1 << TX_PIN))
#define GET_RX_PIN() (PIND & (1 << RX_PIN))


/*! \brief  Type defined enumeration holding software UART's state.
 *
 */
typedef enum
{
    IDLE,                                       //!< Idle state, both transmit and receive possible.
    TRANSMIT,                                   //!< Transmitting byte.
    RECEIVE,                                    //!< Receiving byte.
    DATA_PENDING                                //!< Byte received and ready to read.
	
}AsynchronousStates_t;

static volatile AsynchronousStates_t state;     //!< Holds the state of the UART.
static volatile unsigned char SwUartTXData;     //!< Data to be transmitted.
static volatile unsigned char SwUartTXBitCount; //!< TX bit counter.
static volatile unsigned char SwUartRXData;     //!< Storage for received bits.
static volatile unsigned char SwUartRXBitCount; //!< RX bit counter.

void softuart_init();
void softuart_putc(const unsigned char c);
void softuart_putstr( const char *data );

#endif /* SOFTUART_H_ */
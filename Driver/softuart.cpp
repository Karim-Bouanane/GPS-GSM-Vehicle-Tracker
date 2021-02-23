/*
 * softuart.c
 *
 * Created: 10/02/2021 23:00:46
 *  Author: Karim Bouanane
 */


#include <avr/interrupt.h>
#include <avr/io.h>
#include "softuart.h"
#include <avr/delay.h>

/*! \brief  Function to initialize the software UART.
 *
 *  This function will set up pins to transmit and
 *  receive on. Control of Timer0 and External interrupt 0.
 *
 *  \param  void
 *
 *  \retval void
 */
void softuart_init()
{
	// PORT
	PORTD |= ( 1 << RX_PIN );       // RX_PIN is input, tri-stated.
	DDRD |= ( 1 << TX_PIN );        // TX_PIN is output.
	SET_TX_PIN( );                  // Set the TX line to idle state.
	
	// Timer0
	DISABLE_TIMER_INTERRUPT( );
	TCCR = 0x00;					// Init.
	TCCR_P = 0x00;					// Init.
	TCCR |= (1 << WGM01);			// Timer in CTC mode.
	TCCR_P |=  PRESCALAR_64;        // Divide by 8 prescaler.
	
	// External interrupt
	EXT_ICR = 0x00;                  // Init.
	EXT_ICR |= ( 1 << ISC01 );       // Interrupt sense control: falling edge.
	ENABLE_EXTERNAL0_INTERRUPT( );   // Turn external interrupt on.

	//Internal State Variable
	state = IDLE;    
	
	PORTB |= (1<<5);
	_delay_ms(400);
	PORTB ^= (1<<5);
	_delay_ms(400);
	
		
	// 	Init tx, rx pin
    // 	Init Timer 0
    // 	Init external int0
    // 	State = idle
}

/*! \brief  External interrupt service routine.
 *
 *  The falling edge in the beginning of the start
 *  bit will trig this interrupt. The state will
 *  be changed to RECEIVE, and the timer interrupt
 *  will be set to trig one and a half bit period
 *  from the falling edge. At that instant the
 *  code should sample the first data bit.
 *
 *  \note  initSoftwareUart( void ) must be called in advance.
 */
ISR(INT0_vect)
{
  state = RECEIVE;                  // Change state

  DISABLE_EXTERNAL0_INTERRUPT( );   // Disable interrupt during the data bits.

  DISABLE_TIMER_INTERRUPT( );       // Disable timer to change its registers.
  
  TCCR_P &= ~ PRESCALAR_64;         // Reset prescaler counter.

  TCNT0 = INTERRUPT_EXEC_CYCL;      // Clear counter register. Include time to run interrupt rutine.

  TCCR_P |= PRESCALAR_64;			// Start prescaler clock.

  OCR = TICKS2WAITONE_HALF;         // Count one and a half period into the future.

  SwUartRXBitCount = 0;             // Clear received bit counter.
  
  CLEAR_TIMER_INTERRUPT( );         // Clear interrupt bits
  
  ENABLE_TIMER_INTERRUPT( );        // Enable timer0 interrupt on again
}

ISR(TIMER0_COMPA_vect){
	
	switch(state){
		case TRANSMIT :
			// Output the TX buffer.
			if( SwUartTXBitCount < 8 ) {
				if( SwUartTXData & 0x01 ) {           // If the LSB of the TX buffer is 1:
					SET_TX_PIN();                     // Send a logic 1 on the TX_PIN.
				}
				else {                                // Otherwise:
					CLEAR_TX_PIN();                   // Send a logic 0 on the TX_PIN.
				}
				SwUartTXData = SwUartTXData >> 1;     // Bitshift the TX buffer and
				SwUartTXBitCount++;                   // increment TX bit counter.
			}

			//Send stop bit.
			else {
				SET_TX_PIN();                         // Output a logic 1.
				DISABLE_TIMER_INTERRUPT( );           // Stop the timer interrupts.
				state = IDLE;                         // Go back to idle.
				ENABLE_EXTERNAL0_INTERRUPT( );        // Enable reception again.
			}
		break;
		
		case RECEIVE :
		    OCR = TICKS2WAITONE;					// Count one period after the falling edge is trigged.
													//Receiving, LSB first.
		    if( SwUartRXBitCount < 8 ) {
			    SwUartRXBitCount++;
			    SwUartRXData = (SwUartRXData>>1);   // Shift due to receiving LSB first.
			    if( GET_RX_PIN( ) != 0 ) {
				    SwUartRXData |= 0x80;           // If a logical 1 is read, let the data mirror this.
			    }
		    }

		    //Done receiving
		    else {
			    state = DATA_PENDING;               // Enter DATA_PENDING when one byte is received.
			    DISABLE_TIMER_INTERRUPT( );         // Disable this interrupt.
			    EXT_IFR |= (1 << INTF0 );           // Reset flag not to enter the ISR one extra time.
			    ENABLE_EXTERNAL0_INTERRUPT( );      // Enable interrupt to receive more bytes.
		    }
		break;
		
		// Unknown state.
		default:
			state = IDLE;                           // Error, should not occur. Going to a safe state.

	}
}

void softuart_putc(const unsigned char c)
{
  while( state != IDLE ) ;	// Don't send while busy receiving or transmitting.
  	
  state = TRANSMIT;
  DISABLE_EXTERNAL0_INTERRUPT( );	// Disable reception.
  SwUartTXData = c;					// Put byte into TX buffer.
  SwUartTXBitCount = 0;
  
  TCCR_P &= ~ PRESCALAR_64;         // Reset prescaler counter.
  TCNT0 = 0;                        // Clear counter register.
  TCCR_P |= PRESCALAR_64;           // CTC mode. Start prescaler clock.

  CLEAR_TX_PIN();					// Clear TX line...start of preamble.

  ENABLE_TIMER_INTERRUPT( );        // Enable interrupt
	
}

void softuart_putstr( const char *data )
{
	while( *data != '\0' )
		softuart_putc( *data++ );
	
}
	
    // 	state = idle ?
    // 	disable external interrupt (to disable reception when transmitting)
    // 	data = c
    // 	txbit count = 0
    // 	reset counter timer 0
    // 	set compare value timer 0
    // 	clear tx line
    // 	enable timer interrupt
    //
    // 	set the correct baud timer/counter0
    // 	output the start bit

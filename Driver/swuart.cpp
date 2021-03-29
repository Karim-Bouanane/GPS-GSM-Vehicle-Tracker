/*
 * softuart.cpp
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 *
 */

#include <avr/io.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>

#include "swuart.h"
#include "Timer.h"

// Static variables
uint16_t SWUART::bit_period;
uint16_t SWUART::bit_half_period;

bool ready_to_read = false;

SWUART::SWUART(Timer *timer) : swuartTime(timer)
{
}

void SWUART::setBaud(uint32_t baud)
{
    /* 
		Division by four in the bit period variable is because
		delay_loop_2 executes 4 CPU cycles per iteration
	*/

    bit_period = (F_CPU / baud) / 4UL;
    bit_half_period = bit_period / 2;
}

void SWUART::init()
{
    init(9600); // Initialize with default baud 9600
}

void SWUART::init(uint32_t baud)
{

    // pin receiver
    DDRD &= ~_BV(RX_PIN); // RX_PIN as input
    PORTD |= _BV(RX_PIN); // RX_PIN mode tri-stated

    // pin transmitter
    SET_TX_PIN();        // Set the TX line to idle state
    DDRD |= _BV(TX_PIN); // TX_PIN as output

    setBaud(baud); // Set the desired baud

    ENABLE_INT0_FALLINGEDGE();
    sei(); // enable global interrupt
}

inline void DebugPulse(uint8_t count)
{
    while (count--)
    {
        PORTD |= _BV(7);
        PORTD &= ~_BV(7);
    }
}

void SWUART::send(char data)
{
    cli(); // turn off interrupts for a clean txmit

    // Start Bit
    CLEAR_TX_PIN();
    _delay_loop_2(bit_period);

    // Write bit per bit
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & 0x01)  // If the LSB of the TX buffer is 1
            SET_TX_PIN(); // Send a logic 1 on the TX_PIN
        else
            CLEAR_TX_PIN(); // Send a logic 0 on the TX_PIN

        data = data >> 1;          // Bitshift the TX buffer and
        _delay_loop_2(bit_period); // wait one period
    }

    // Stop bit
    SET_TX_PIN(); // Set the TX line to idle state
    _delay_loop_2(bit_period);

    sei();
}

void SWUART::sendString(const char *message)
{
    while (*message != '\0')
        send(*message++);
}

void SWUART::sendString(const char *message, uint16_t len)
{
    while (len-- && *message != '\0')
        send(*message++);
}

inline void SWUART::recv(char *data)
{
    char recv = 0;

    _delay_loop_2(bit_half_period); // Center in the start bit so as to read next bit form center

    for (uint8_t i = 0; i < 8; i++)
    {
        _delay_loop_2(bit_period);

        recv >>= 1; // we receive byte from LSB to MSB

        if (PIND & _BV(RX_PIN))
            recv |= 0x80; // we receive byte from LSB to MSB
    }

    *data = recv; // return read data

    _delay_loop_2(bit_period); // skip the stop bit
}

void SWUART::read(char *data)
{
    ENABLE_EXTERNAL0_INTERRUPT(); // enable interrupt when start reading

    while (ready_to_read == false) // I know it's not clean code, I will enhance it in the future
        ;

    ready_to_read = false;

    recv(data); // receive data of 1 Byte

    EIFR |= _BV(INTF0); // ignore any interrupt request in INT0
}

bool SWUART::read(char *data, uint32_t timeout)
{
    ENABLE_EXTERNAL0_INTERRUPT(); // enable interrupt when start reading

    uint32_t prev = swuartTime->now();

    while (ready_to_read == false) // I know it's not clean code, I will enhance it in the future
    {
        if (swuartTime->now() - prev > timeout) // be sure not exceed the timeout
            return false;
    }

    ready_to_read = false;

    recv(data); // receive data of 1 Byte

    EIFR |= _BV(INTF0); // ignore any interrupt request in INT0

    return true;
}

ISR(INT0_vect)
{
    ready_to_read = true;
    DISABLE_EXTERNAL0_INTERRUPT();
}

size_t SWUART::readString(char *buff, size_t len, uint32_t timeout)
{
    size_t i = len;

    while (len--) // decrement until reaching 0
    {
        if (read(buff, timeout) == false)
            return 0; // timeout is reached

        if (*buff == '\r' || *buff == '\n') // string terminator
            break;

        ++buff; // move to the next address
    }

    *buff = 0;      // close array
    return i - len; // return number of read characters
}

size_t SWUART::readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout)
{
    size_t i = len;

    while (len--) // decrement until reaching 0
    {
        if (read(buff, timeout) == false)
            return 0; // timeout is reached

        if (*buff == terminator) // string terminator
            break;

        ++buff; // move to the next address
    }

    *buff = 0;      // close array
    return i - len; // return number of read characters
}
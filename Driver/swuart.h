/*
 * softuart.h
 *
 * 
 * Author: Karim Bouanane
 * Hardware: ATMEGA328P
 */

#ifndef SOFTUART_H_
#define SOFTUART_H_

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

#include "Timer.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Some IO, timer and interrupt specific defines.

#define ENABLE_EXTERNAL0_INTERRUPT() (EIMSK |= _BV(INT0))
#define DISABLE_EXTERNAL0_INTERRUPT() (EIMSK &= ~_BV(INT0))
#define ENABLE_INT0_FALLINGEDGE() (EICRA |= _BV(ISC01))

#define TX_PIN PORTD3 // Transmit data pin
#define RX_PIN PORTD2 // Receive data pin, must be INT0
#define SET_TX_PIN() (PORTD |= _BV(TX_PIN))
#define CLEAR_TX_PIN() (PORTD &= ~_BV(TX_PIN))

#ifndef MAX_RX_BUFF
#define MAX_RX_BUFF 50
#endif

class SWUART
{
public:
    Timer *swuartTime;
    static uint16_t bit_period;
    static uint16_t bit_half_period;

public:
    SWUART(Timer *timer);

    void init();
    void init(uint32_t baud);
    void setBaud(uint32_t baud);
    void send(char data);
    void sendString(const char *message);
    void sendString(const char *message, uint16_t len);
    void read(char *data);
    bool read(char *data, uint32_t timeout);
    size_t readString(char *buff, size_t len, uint32_t timeout = 1000);
    size_t readStringUntil(char terminator, char *buff, size_t len, uint32_t timeout = 1000);

private:
    inline void recv(char *data);
};

#endif /* SOFTUART_H_ */
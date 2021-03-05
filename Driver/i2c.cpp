/*
 * i2c.cpp
 *
 * Blocking i2c Driver
 * My code is based on this simple library :
 * https://github.com/sebig3000/ATmega328P/blob/master/src/twi.c
 *
 * Author	: Karim Bouanane
 * Hardware : ATMEGA328P
 * 
 */

#include <avr/io.h>	  // Hardware registers
#include <util/twi.h> // TWI status masks
#include "i2c.h"

#define _BV(x) (uint8_t)(1 << x)
#define F_CPU 16000000

#define TWI_ADDRESS_W(addr) (((addr) << 1) & ~0x01) // add write mode with slave address
#define TWI_ADDRESS_R(addr) (((addr) << 1) | 0x01)	// add read mode with slave address

void I2C::init(void)
{

	//DDRC &= ~ (_BV(5) | _BV(4));  // set SDA-SCL pins as input
	//PORTC |= (_BV(5) | _BV(4)); // activate pull up in SDA-SCL pins
	
	setClock(100000UL); // initialize i2c frequency
	TWCR = _BV(TWEN); // enable i2c module, acks, and i2c interrupt
}

enum{
	I2C_FREQ_100,
	I2C_FREQ_200,
	I2C_FREQ_400	
};

bool I2C::setClock(uint32_t frequency)
{
	/* The SCL frequency is generated according to the following equation:
	SCL frequency =  Cpu clock freq / (16 + 2*(TWBR) * Prescalar value
	
	set prescaler so that the TWBR value is as large as possible
	and at least 10 (frequency error below 5%)
	*/
	
	uint16_t bitRate = ((F_CPU / frequency) - 16) / 2;
	uint16_t bitRate_presc_1 = bitRate;
	uint16_t bitRate_presc_4 = bitRate / 4; 
	uint16_t bitRate_presc_16 = bitRate / 16; 
	uint16_t bitRate_presc_64 = bitRate / 64;
	
	if( bitRate_presc_1 >= 10 && bitRate_presc_1 <= 0xFF )
	{
		TWBR = bitRate_presc_1;
	}
	else if( bitRate_presc_4 >= 10 && bitRate_presc_4 <= 0xFF )
	{
		TWBR = bitRate_presc_4;
		TWSR |= _BV(TWPS0);
	}
	else if( bitRate_presc_16 >= 10 && bitRate_presc_16 <= 0xFF )
	{
		TWBR = bitRate_presc_16;
		TWSR |= _BV(TWPS1);
	}
	else if( bitRate_presc_64 >= 10 && bitRate_presc_64 <= 0xFF )
	{
		TWBR = bitRate_presc_64;
		TWSR |= _BV(TWPS1) | _BV(TWPS0);
	}
	else
		return 0;
	
	return 1;

}

void I2C::disable(void)
{
	TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA)); // disable i2c module, acks, and i2c interrupt
	PORTC &= ~(_BV(5) | _BV(4));				  // deactivate internal pullups for SDA SCL pins.
}

static void waitForComplete(void)
{
	while (~TWCR & _BV(TWINT)) // Wait until interrupt occur
		;
}

bool I2C::start(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); // Enable TWI, generate start

	waitForComplete(); // Wait until TWI finish its current job

	return TW_STATUS == TW_START ||
		   TW_STATUS == TW_REP_START; // Check weather START transmitted or not

	/*A special case occurs when a new START condition is
	issued between a START and STOP condition. This is referred
	to as a REPEATED START condition, and is used when the master
	wishes to initiate a new transfer without relinquishing control
	of the bus. After a REPEATED START, the bus is considered
	busy until the next STOP.*/
}

void I2C::setSlaveAddress(uint8_t address)
{
	this->address = address;
}

bool I2C::masterReadMode()
{
	TWDR = TWI_ADDRESS_R(address); // Write slave address with read mode
	TWCR = _BV(TWINT) | _BV(TWEN); // Enable TWI & clear interrupt flag

	waitForComplete(); // Wait for slave to respond

	return TW_STATUS == TW_MR_SLA_ACK; // Return if slave respond
}

bool I2C::masterWriteMode()
{
	TWDR = TWI_ADDRESS_W(address); // Write slave address with write mode
	TWCR = _BV(TWINT) | _BV(TWEN); // Enable TWI & clear interrupt flag

	waitForComplete(); // Wait for slave to respond

	return TW_STATUS == TW_MT_SLA_ACK; // Return if slave respond
}

bool I2C::connectTo(uint8_t address)
{

	setSlaveAddress(address); // save slave address

	if (!start()) // Generate start condition
		return 0; // Start condition fail

	/* Between a START and a STOP condition, the bus is considered
	busy, and no other master should try to seize control of the bus */

	return masterWriteMode(); // Return if connection with slave was established
}

void I2C::stop(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); // Enable TWI, generate stop
	while (TWCR & (1 << TWSTO))
		; // Wait until stop condition execution
}

bool I2C::write(uint8_t data)
{
	TWDR = data;				   // Copy data in TWI data register
	TWCR = _BV(TWINT) | _BV(TWEN); // Enable TWI & clear interrupt flag

	waitForComplete(); // Wait for write operation to complete

	return TW_STATUS == TW_MT_DATA_ACK; // Return if data was sent successfully
}

bool I2C::readAck(uint8_t *data)
{
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA); // Enable TWI, generation of ack

	waitForComplete(); // Wait for read operation to complete

	*data = TWDR; // Sava received data

	return TW_STATUS == TW_MR_DATA_ACK; // Return if data was correctly read
}

bool I2C::readNAck(uint8_t *data)
{
	TWCR = _BV(TWEN) | _BV(TWINT); // Enable TWI, generation of ack

	waitForComplete(); // Wait for read operation to complete

	*data = TWDR; // Sava received data

	return TW_STATUS == TW_MR_DATA_NACK; // Return if data was correctly read
}

bool I2C::writeToRegister(uint8_t regAddr, uint8_t data)
{
	if (start() &&			 // Send start to change mode to write
		masterWriteMode() && // Master in write mode
		write(regAddr) &&	 // Write the address of desired register
		write(data))		 // Write given data
	{
		stop();	  //	Release I2C bus
		return 1; //  Data sent successfully
	}

	stop();	  // Release i2c bus
	return 0; // Sending data fail
}

bool I2C::readFromRegister(uint8_t regAddr, uint8_t *data)
{

	if (start() &&			 // Send start to change mode to write
		masterWriteMode() && // Master in write mode
		write(regAddr) &&	 // Write the address of desired register
		start() &&			 // Send start to change mode to read
		masterReadMode() &&	 // Master in read mode
		readNAck(data))		 // Write given data
	{
		stop();	  //	Release I2C bus
		return 1; //  Data read successfully
	}

	stop();	  //	Release I2C bus
	return 0; //  Reading data fail
}

uint16_t I2C::writeStartFromRegister(uint8_t regAddr, uint8_t* data, uint16_t quantity)
{

	if (start() != 1 ||			  // Send start to change mode to write
		masterWriteMode() != 1 || // Master in write mode
		write(regAddr) != 1)	  // Write the address of desired register
	{
		stop();	  //	Release I2C bus
		return 0; //  Failed to start
	}

	uint16_t i = quantity; // Help us to calculate how many data was written correctly

	while (i)
	{
		if (!write(*data++))
			break;
		i--;
	}

	stop();
	return quantity - i;
}

uint16_t I2C::readStartFromRegister(uint8_t regAddr, uint8_t *data, uint16_t quantity)
{

	if (start() != 1 ||			  // Send start to change mode to write
		masterWriteMode() != 1 || // Master in write mode
		write(regAddr) != 1 ||	  // Write the address of desired register
		start() != 1 ||			  // Resend start to change mode to read
		masterReadMode() != 1)	  // Verify if connection with slave was established
	{
		stop();	  //	Release I2C bus
		return 0; //  Failed to start
	}

	uint16_t i = quantity; // Help us to calculate how many data was correctly read

	while (i)
	{
		if (i == 1) // Last data to read
		{
			readNAck(data); // Slave will not send next data when he receive Not Acknowledge signal
			break;
		}

		if (!readAck(data++)) // Verify if data was correctly read
			break;
		i--;
	}

	stop();			// release i2c BUS
	return quantity - i; // Return how many data was successfully written
}

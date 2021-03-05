/*
 * i2c.h
 *
 * Blocking i2c Driver
 *
 * Author	: Karim Bouanane
 * Hardware : ATMEGA328P
 *
 */

#ifndef I2C_H_
#define I2C_H_

class I2C
{
private:
	uint8_t address;

public:

	void init(void);
	void disable(void);
	bool masterReadMode();
	bool masterWriteMode();
	void setSlaveAddress(uint8_t address);
	uint8_t getSlaveAddress() { return address; };
	bool connectTo(uint8_t address);
	bool setClock(uint32_t frequency);
	bool start(void);
	void stop(void);
	bool write(uint8_t data);
	bool readAck(uint8_t *data);
	bool readNAck(uint8_t *data);
	bool writeToRegister(uint8_t regAddr, uint8_t data);
	uint16_t writeStartFromRegister(uint8_t regAddr, uint8_t* data, uint16_t quantity);
	bool readFromRegister(uint8_t regAddr, uint8_t *data);
	uint16_t readStartFromRegister(uint8_t regAddr, uint8_t* data, uint16_t quantity);
};

#endif /* I2C_H_ */
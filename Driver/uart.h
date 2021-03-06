/*
 * uart.cpp
 *
 * Created: 08/02/2021 16:36:32
 *  Author: Karim Bouanane
 */

#ifndef UART_H_
#define UART_H_

class UART
{
public:
	void init();
	void init(uint32_t baud);
	void setBaud(uint32_t baud);
	unsigned char readChar();
	void sendChar(unsigned char);
	void sendString(const char *message);
	void sendString(const char *message, int len);
	void readString(unsigned char *buff, int len);
};

#endif /* UART_H_ */

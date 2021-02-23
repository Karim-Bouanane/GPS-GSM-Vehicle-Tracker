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
	unsigned char readChar();
	void sendChar(unsigned char);
	void sendString(unsigned char* buff, int len);
	void readString(unsigned char* buff, int len);

};



#endif /* UART_H_ */

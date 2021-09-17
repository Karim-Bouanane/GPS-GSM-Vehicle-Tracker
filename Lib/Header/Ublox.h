/*
 * Ublox.h
 * Ublox-M8
 *
 * Author: Karim Bouanane
 * Hardware : Atmega328p
 */ 


#ifndef UBLOX_H_
#define UBLOX_H_

#include "uart.h"


typedef enum
{
	// module connection
	GPS_SUCCESS_REPLY = 1,
	GPS_DISCONNECTED,
	GPS_RESTART_FFAIL,
	
}GPSCode;


typedef enum
{
	NO_MESSAGE = 0x0000,
	NAV_POSLLH = 0x0102,
	NAV_STATUS = 0x0103
	
} MssgType;

	
class Ublox
{
	protected:
	
		UART serialGPS;
	
	public:
		
		uint8_t reset();
		uint8_t isConnected();
		uint8_t enableMessage(MssgType type);
		uint8_t sendReceive(const char*cmd, size_t cmdLength, const char* resp, size_t respLength, uint32_t timeout);
		
		// control the power state of a GNSS module
		uint8_t sleep(); 
};



#endif /* UBLOX_H_ */
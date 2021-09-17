/*
 * Ublox.cpp
 *
 * Ublox-M8
 *
 * Author: Karim Bouanane
 * Hardware : Atmega328p
 */ 

#include "Ublox.h"


/**** Definitions ****/

#define CHIP_ID_CMD_SIZE sizeof(chipID_cmd)
#define CHIP_ID_RESP_SIZE sizeof(chipID_resp)

#define RESET_CMD_SIZE sizeof(reset_cmd)
#define RESET_RESP_SIZE sizeof(reset_resp)

#define NAV_POSSLH_CMD_SIZE sizeof(nav_posllh_cmd)
#define NAV_POSSLH_RESP_SIZE sizeof(nav_posllh_resp)

#define NAV_STATUS_CMD_SIZE sizeof(nav_status_cmd)
#define NAV_STATUS_RESP_SIZE sizeof(nav_status_resp)


/**** Constants ****/

const char chipID_cmd[] = {0xB5,0x62,0x27,0x03,0x00,0x00,0x2A,0xA5};
const char chipID_resp[] = {0xB5,0x62,0x27,0x03}; // we don't need complete response to verify the connection

const char reset_cmd[] = {0xB5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x00, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x44};
const char reset_resp[] = {'$', 'G', 'N', 'T', 'X', 'T'};
	
const char nav_posllh_cmd[] = {0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x02, 0x01, 0x0E, 0x47};
const char nav_posllh_resp[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x01, 0x0F, 0x38};

const char nav_status_cmd[] = {0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x03, 0x01, 0x0F, 0x49};
const char nav_status_resp[] = {0xB5, 0x62, 0x01, 0x03};
	

// Debugging function

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}


uint8_t Ublox::sendReceive(const char* cmd, size_t cmdLength, const char* resp, size_t respLength, uint32_t timeout)
{
	serialGPS.sendBytes(cmd, cmdLength);
	return serialGPS.find(resp, respLength, timeout);
}


uint8_t Ublox::isConnected()
{	
	if(sendReceive(chipID_cmd, CHIP_ID_CMD_SIZE, chipID_resp, CHIP_ID_RESP_SIZE, 4000) == false)
		return GPS_DISCONNECTED;
	
	return GPS_SUCCESS_REPLY;
}


uint8_t Ublox::reset()
{
	if(sendReceive(reset_cmd, RESET_CMD_SIZE, reset_resp, RESET_RESP_SIZE, 4000) == false)
		return GPS_RESTART_FFAIL;
		
	return GPS_SUCCESS_REPLY;
}


uint8_t Ublox::enableMessage(MssgType type)
{
	switch(type)
	{
		case NAV_POSLLH:
		
			return sendReceive(nav_posllh_cmd, NAV_POSSLH_CMD_SIZE, nav_posllh_resp, NAV_POSSLH_RESP_SIZE, 2000);
				
		break;
		
		case NAV_STATUS:
		
			return sendReceive(nav_status_cmd, NAV_STATUS_CMD_SIZE, nav_status_resp, NAV_STATUS_RESP_SIZE, 2000);
	
		break;
		
		default:
		break;	
	}
	
	return false;
}
/*
 * UbxGPS.cpp
 *
 * GPS library providing UBX protocol parsing for two type of sentences : NAV_POSLLH, NAV_STATUS
 * I took inspiration from this library :
 * https://github.com/emlid/Navio/blob/master/C%2B%2B/Navio/Ublox.cpp
 * 
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */

#include "UBXGPS.h"


// Debugging function

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}


UBXGPS::UBXGPS() 
	: validFixCount(0)
	, invalidFixCount(0)
	, failedChecksumCount(0)
	, passedChecksumCount(0)
{
}


/**** Initialization ****/

void UBXGPS::initSerial()
{
	serialGPS.init(9600);
}


/**** Location ****/

uint8_t UBXGPS::waitValidLocation()
{
	MssgType type;
	
	while(1)
	{
		type = getGPSMessage();
		
		if (type == NO_MESSAGE)					// gps module not sending any messages
		{
			if(isConnected() == false)			// check module connection
			{
				return GPS_DISCONNECTED;
			}
			else if (reset() == false)			// try restarting the module
			{
				return GPS_RESTART_FAIL;
			}
		}
		else if (type == NAV_STATUS && gpsFixOK == true)	// wait for NAV_STATUS message and check gpsFix
		{
			if(getGPSMessage() == NAV_POSLLH)				// if Fix is acquired, extract location from NAV_POSLLH message
				return LOCATION_FOUND;
		}
	}
}


long UBXGPS::getLatitude()
{
	return latitude;
}


long UBXGPS::getLongitude()
{
	return longitude;
}


char* UBXGPS::getStrLatitude()
{
	ltoa(latitude, gpsCoord.lat_str, 10);
	return gpsCoord.lat_str;
}


char* UBXGPS::getStrLongitude()
{
	ltoa(longitude, gpsCoord.lng_str, 10);
	return gpsCoord.lng_str;
}


Coord* UBXGPS::getGPSCoord()
{
	return &gpsCoord;
}


/**** Parsing ****/

MssgType UBXGPS::getGPSMessage()
{
	resetState();
	
	char data;
	bool validMssg;
	
	do
	{
		if(serialGPS.read(&data, 20000) == false)	// read one character
			return NO_MESSAGE;						// problem with GPS module
		
		validMssg = encode(data);					// construct terms from read characters
													// until reaching checksum terms
	} while (validMssg != true);
	
	return (MssgType)id;
}


bool UBXGPS::encode(char data)
{
	switch (state)
	{
		case Sync1:
			resetState();
			if (data == 0xb5)
				state = Sync2;
		
		return false;
		
		case Sync2:
			if (data == 0x62)
				state = Class;
			else
				resetState();
		
		return false;
		
		case Class:
			id = (uint16_t)data << 8;
			state = ID;
		
		break;

		case ID:
			id += data;
			state = Length1;
		
		break;

		case Length1:
			payload_length = data;
			state = Length2;
		
		break;

		case Length2:
			payload_length += data << 8;
			state = Payload;
		
		break;

		case Payload:
			if(offset == 0)
				resetTerms();
		
			termHandler(data);

			offset++;
		
			if(offset >= payload_length)
				state = CK_A;
		
		break;

		case CK_A:
		
			if(data != calCK_A)
			{
				failedChecksumCount++;
				resetState();
			}
			else
			{
				state = CK_B;
			}

		return false;
		
		case CK_B:
		
			if(data != calCK_B)
			{
				failedChecksumCount++;
				resetState();
			
				return false;
			}
			else
			{
				passedChecksumCount++;
				state = Done;
			
				return true;
			}
		
		case Done:
		default:
			resetState();
		break;
	}
	
	calCK_A += data;
	calCK_B += calCK_A;
	
	return false;
}


/**** Private Methods ****/

void UBXGPS::resetState()
{
	offset = 0;
	calCK_A = 0;
	calCK_B = 0;
	state = Sync1;
}


void UBXGPS::resetTerms()
{
	gpsFixOK = false;
	latitude =  0;
	longitude = 0;
}


void UBXGPS::termHandler(char data)
{
	switch(id)
	{
		case NAV_POSLLH:
			
			if( offset >= 12)
			{
				// ignore those fields
				// Height above Ellipsoid
				// Height above mean sea level
				// Horizontal Accuracy Estimate
				// Vertical Accuracy Estimate
				break;
			}
			else if( offset >= 8)	// get 4 bytes for the latitude starting from the 8th byte
			{
				uint16_t i = offset - 8; 
				latitude += (long)(data) << i*8;
			}
			else if( offset >= 4)	// get 4 bytes for the longitude starting from the 4th byte
			{
				uint16_t i = offset - 4; 
				longitude += (long)(data) << i*8;
			}
			
			// ignore iTOW field
		break;
			
		case NAV_STATUS:
			
			if( offset >= 5)
			{
				// ignore those fields
				// flags
				// fixStat
				// flags2
				// ttff
				// msss
				break;
			}
			else if ( offset == 4)	// get 1 byte for the gpsFix in position 4
			{	
				// for a fix to be valid the data value need to be within the following interval 
				//if(data >= 0x01  && data < 0x05)
				if(data > 0x01  && data < 0x05) 
				{
					validFixCount++;
					gpsFixOK = true;
				}
				else
				{
					invalidFixCount++;
				}
			}

		break;
			
		default:
			break;
	}
	
}
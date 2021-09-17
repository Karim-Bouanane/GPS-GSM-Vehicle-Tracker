/*
 * UbxGPS.h
 *
 * GPS library providing UBX protocol parsing for two type of sentences : NAV_POSLLH, NAV_STATUS
 * I took inspiration from this library :
 * https://github.com/emlid/Navio/blob/master/C%2B%2B/Navio/Ublox.cpp
 *
 * Author: Karim Bouanane
 * Hardware : ATMEGA328P
 */ 

#ifndef UBXGPS_H_
#define UBXGPS_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "Ublox.h"

#define	LOCATION_FOUND		2
#define GPS_RESTART_FAIL	3


typedef struct
{
	char lat_str[10];
	char lng_str[10];
	
} Coord;


typedef enum
{
	Sync1 = 0,
	Sync2,
	Class,
	ID,
	Length1,
	Length2,
	Payload,
	CK_A,
	CK_B,
	Done
	
}State;

	
class UBXGPS : public Ublox
{

private:	// private variables
	
	// parsing state variables
	uint16_t id;
	State state;                // Current scanner state
	bool gpsFixOK;				// 
	uint16_t offset;			// Indicates payload buffer offset
	uint16_t payload_length;	// Length of current message payload
	uint8_t calCK_A, calCK_B;	// 
	
	// location
	Coord gpsCoord;
	long longitude;			// deg      Longitude (1e-7)
	long latitude;			// deg      Latitude (1e-7)
				
	// statistics
	uint32_t validFixCount;
	uint32_t invalidFixCount;
	uint32_t failedChecksumCount;
	uint32_t passedChecksumCount;

public:		// public methods

	UBXGPS();
	
	// Initialization
	void initSerial();
	
	// Location
	uint8_t waitValidLocation();
	long getLatitude();
	long getLongitude();
	char* getStrLatitude();
	char* getStrLongitude();
	Coord* getGPSCoord();
	
	// Parsing
	MssgType getGPSMessage();
	bool encode(char data);
	
private:	// private methods

	void resetState();
	void resetTerms();
	void termHandler(char data);
};

#endif /* UBXGPS_H_ */
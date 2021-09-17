/*
 * main.cpp
 *
 * 
 * Author : Karim Bouanane
 */

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include "ErrorHandler.h"
#include "UBXGPS.h"
#include "GPRS.h"


/** Definitions **/

#define SERVER_URL		"https://karim-gps.glitch.me/updateLoc"
#define CONTENT_TYPE	"application/x-www-form-urlencoded"
#define APN_ORANGE		"internet.orange.ma"
#define APN_IAM			"www.iamgrps1.ma"

#define GPS_GPRS_DISCONNECTED	1


// Debugging function

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}


UBXGPS gps;
GPRS gprs;

char httpData[30];

int main()
{
	//DDRD |= _BV(7);	// this pin is used by the logic analyzer device for debugging 
	
	// Initialize serial communication
	
	gps.initSerial();
	gprs.initSerial();
	
	
	// Verify module connection
	
	uint8_t httpStatus;
	uint8_t gprsStatus;
	uint8_t gpsStatus;
	
	gprsStatus = gprs.isConnected();			
	gpsStatus = gps.isConnected();
	
	if(gprsStatus == GPRS_DISCONNECTED && gpsStatus == GPS_DISCONNECTED)
	{
		errorHandler(BOTH_MODULE, GPS_GPRS_DISCONNECTED);
	}
	else if(gpsStatus == GPS_DISCONNECTED)
	{
		errorHandler(GPS_MODULE, GPS_DISCONNECTED);
	}
	else if(gprsStatus == GPRS_DISCONNECTED)
	{
		errorHandler(GPRS_MODULE, GPRS_DISCONNECTED);
	}	
	
	
	// Wait gprs module to be ready
	
	gprsStatus = gprs.waitReady();
	
	if(gprsStatus != GPRS_SUCCESS_REPLY )
	{
		errorHandler(GPRS_MODULE, gprsStatus);
	}
	
	
	// Initialize module
	
	gprsStatus = gprs.activateGPRS(APN_IAM);
	
	if(gprsStatus != GPRS_SUCCESS_REPLY)			// connect to the internet
	{
		errorHandler(GPRS_MODULE, gprsStatus);
	}
	
	
	// GPS is already configure by u-center software to generate these messages
	
	// gps.enableMessage(NAV_STATUS);
	// gps.enableMessage(NAV_POSLLH);
	
	
	// Loop
	
	while(1)
	{
		// Acquire Location
		
		gpsStatus = gps.waitValidLocation();
		
		if (gpsStatus != LOCATION_FOUND)
		{
			errorHandler(GPS_MODULE, gpsStatus);
		}
		
		// Construct URL Request
		
		strcpy(httpData, "lat=");
		strcat(httpData, gps.getStrLatitude());
		strcat(httpData, "&lng=");
		strcat(httpData, gps.getStrLongitude());
		
		// Send HTTP Post Request to server
		
		httpStatus = gprs.send_HTTP_POSTRequest(SERVER_URL, CONTENT_TYPE, httpData, 5);
		
		if(httpStatus != GPRS_SUCCESS_REPLY)
			errorHandler(GPRS_MODULE, httpStatus);
		
	}
	
}
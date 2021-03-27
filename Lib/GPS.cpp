/*
 * GPS.cpp
 *
 * GPS library providing NMEA parsing for one sentence type : GNGLL
 * It is based on the famous library TinyGPS++
 * Author: Karim Bouanane
 * Hardware : Atmega328p
 * 
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart.h"
#include "GPS.h"

#define _GNGLLterm "GNGLL"

GPS::GPS(UART *serialInput) 
	: serialGPS(serialInput)
	, validFixCount(0)
	, invalidFixCount(0)
	, failedChecksumCount(0)
	, passedChecksumCount(0)
{
	term[0] = '\0';
}

void GPS::init(uint32_t baud)
{
	serialGPS->init(baud);
}

bool GPS::isConnected()
{
	// Working on it.
	return true;
}

char *GPS::getPosition()
{
	bool validPosition = false;
	
	do 
	{
		char c = serialGPS->read();	// read one character
		validPosition = encode(c);	// construct terms from read characters 
									// until reaching the parity term
		
	} while (validPosition == false);
	
	return JSON_Coordinate;
}

/* Decode the NMEA protocol
 * This function try to extract
 * each term of the sentence GNGLL
 * That has the following format
 * $GNGLL, Latitude,N/S Indicator,Longitude,E/W Indicator,UTC Time,Status,Mode*checksum
 * Example : $GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*5F
 */
bool GPS::encode(char c)
{

    switch (c)
    {
    case ',':                 // term terminators
        parity ^= (uint8_t)c; // calculate parity for the following part
                              // GNGLL,Latitude,N/S Indicator,Longitude,E/W Indicator,UTC Time,Status,Mode
    case '\r':
    case '\n':
    case '*': // beginning of term checksum
    {
        bool isValidSentence = false;
        if (curTermOffset < sizeof(term))
        {
            term[curTermOffset] = 0; // close the array
            isValidSentence = termHandler();
        }
        ++curTermNumber;
        curTermOffset = 0;
        isChecksumTerm = c == '*';
        return isValidSentence;
    }
    break;

    case '$':				// begin of sentence
        curTermNumber = 0;	// indicate term number in NMEA sentence
        curTermOffset = 0;	// index of position in term array
        parity = 0;			// used for error detection
        sentenceTypeGNGLL = false;
        isChecksumTerm = false;
        isDataValid = false;
        return false;

    default:							// ordinary characters
        if (curTermOffset < sizeof(term) - 1)
            term[curTermOffset++] = c;	// constructing term
        if (!isChecksumTerm)
            parity ^= c;
        return false;
    }

    return false;
}

/* Processes a just-completed term
 * Returns true if new sentence has just passed checksum test and is validated
 */
bool GPS::termHandler()
{
    // If it's the checksum term, and the checksum checks out,
    // set location isValid true
    if (isChecksumTerm)
    {
        uint8_t checksum = 16 * HextoDec(term[0]) + HextoDec(term[1]);
		
        if (checksum != parity) // Checks for errors
        {
			++failedChecksumCount;	// count the sentences that failed in the checksum test
            return false;
        }
		++passedChecksumCount;	// count the sentences that passed in the checksum test
		
		if (isDataValid == false)
		{
			++invalidFixCount; // count invalid locations
			return false;
		}
		++validFixCount;	// count valid locations
        return true;
    }

    // The first term determines the sentence type
    if (curTermNumber == 0)
    {
        if (!strcmp(term, _GNGLLterm))
            sentenceTypeGNGLL = true;
        else
            sentenceTypeGNGLL = false;

        return false;
    }

    // Extract the information contained in terms
    //         term1   , term2       , term3   , term4       , term5  , term6
    // $GNGLL, Latitude,N/S Indicator,Longitude,E/W Indicator,UTC Time,Status, Mode

    if (sentenceTypeGNGLL == true && term[0])
    {
        switch (curTermNumber)
        {
        case 1:
			strcpy(JSON_Coordinate, "{\"lat\":");		// Latitude term
			strcat(JSON_Coordinate, term);
            break;

        case 2:
			strcat(JSON_Coordinate, ",\"S\":");			// South Indicator
			strcat(JSON_Coordinate, &term[0]); 
            break;

        case 3:
				strcat(JSON_Coordinate, ",\"lng\":");	// Longitude term
				strcat(JSON_Coordinate, term);
            break;

        case 4:
			strcat(JSON_Coordinate, ",\"W\":");
            strcat(JSON_Coordinate, &term[0]);			// West Indicator
			strcat(JSON_Coordinate, "}");
            break;

        case 5:											
			// I don't need time so I ignore it
            break;

        case 6:
			//  A: Valid   V: Invalid
            isDataValid = term[0] == 'A';
            break;
        }
    }

    return false;
}

// internal utilities
// Convert the checksum Hexa to Int
int GPS::HextoDec(char a)
{
    if (a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    else if (a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    else
        return a - '0';
}
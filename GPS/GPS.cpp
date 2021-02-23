/*
 * GPS.cpp
 * GPS library provding NMEA parsing from one sentence type GNGLL
 * It is based on the famous library TinyGPS++
 * Author: Karim Bouanane
 */

#include "GPS.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#define _GNGLLterm "GNGLL"

/* Decode the NMEA protocol
 * This function try to extract
 * each term of the sentence GNGLL
 * That has the following format
 * $GNGLL, Latitude,N/S Indicator,Longitude,E/W Indicator,UTC Time,Status,Mode*checksum
 * Exemple : $GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*5F
 */
bool GPS::encode(char c)
{

    switch (c)
    {
    case ',':                 // term terminators
        parity ^= (uint8_t)c; // calculate parity for this whole part
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

    case '$':              // sentence begin
        curTermNumber = 0; // the first term
        curTermOffset = 0; //
        parity = 0;
        curSentenceType = GPS_SENTENCE_OTHER;
        isChecksumTerm = false;
        isStatusValid = false;
        locationValid = true;
        return false;

    default: // ordinary characters
        if (curTermOffset < sizeof(term) - 1)
            term[curTermOffset++] = c;
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
    // set locationisValid true
    if (isChecksumTerm)
    {
        uint8_t checksum = 16 * HextoDec(term[0]) + HextoDec(term[1]);
        if (checksum == parity && isStatusValid) // Check if data is valid
        {
            locationValid = true; // Location is Valid and can be retrieve
            return true;
        }
        return false;
    }

    // The first term determines the sentence type
    if (curTermNumber == 0)
    {
        if (!strcmp(term, _GNGLLterm))
            curSentenceType = GPS_SENTENCE_GNGLL;
        else
            curSentenceType = GPS_SENTENCE_OTHER;

        return false;
    }

    // Extract the information contained in terms
    //         term1   , term2       , term3   , term4       , term5  , term6
    // $GNGLL, Latitude,N/S Indicator,Longitude,E/W Indicator,UTC Time,Status, Mode

    if (curSentenceType != GPS_SENTENCE_OTHER && term[0])
    {
        switch (curTermNumber)
        {
        case 1:
            calculateLatitude(term); // Convert Latitude from Degrees Decimal Minutes
                                     // to Decimal degrees
            break;

        case 2:
            setSIndicator(term[0] == 'S'); // South Indicator
            break;

        case 3:
            calculateLongitude(term); // Convert Longitude from Degrees Decimal Minutes
                                      // to Decimal degrees
            break;

        case 4:
            setWIndicator(term[0] == 'W'); // West Indicator
            break;

        case 5:
            setTime(term); // Extract hours minutes seconds centiseconds
            break;

        case 6:
            setStatus(term); // Set status
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

/* Convert Latitude and Longitude raw data to positioning
 * We need to Convert degrees, minutes to decimal degrees
 * So as to achieve that we will keep the part o degree
 * and try to replace One minute with 1/60 degrees so
 * the equation will equal to dd = d + m/60
*/

void GPS::calculateLatitude(char *term)
{
    double minutes = (double)(atof(term + 2) / 60.0);
    term[2] = 0; // keep the part of DD and truncate the other part
    uint8_t deg = (uint8_t)atoi(term);
    latitude = deg + minutes;
}

void GPS::calculateLongitude(char *term)
{
    double minutes = (double)(atof(term + 3) / 60.0);
    term[3] = 0; // keep the part of DD and truncate the other part
    uint8_t deg = (uint8_t)atoi(term);
    longitude = deg + minutes;
}

/* Include the negative sign for South coordinates */
void GPS::setSIndicator(bool value)
{
    longitude = (value == true) ? -longitude : longitude;
}

/* Include the negative sign for West coordinates */
void GPS::setWIndicator(bool value)
{
    latitude = (value == true) ? -latitude : latitude;
}

/* Status term give info about the validity of data
 * A: Valid
 * V: Invalid
*/
void GPS::setStatus(const char *term)
{
    isStatusValid = term[0] == 'A';
}

// Extract time from Universal Time coordinated format : hhmmss.sss
void GPS::setTime(char *term)
{
    time.centisecond = (uint16_t)atoi(term + 7);
    term[6] = 0;
    time.second = (uint8_t)atoi(term + 4);
    term[4] = 0;
    time.minute = (uint8_t)atoi(term + 2);
    term[2] = 0;
    time.hour = (uint8_t)atoi(term);
}
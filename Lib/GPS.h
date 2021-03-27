/*
 * GPS.h
 *
 * Created: 17/02/2021 18:45:19
 *  Author: Karim Bouanane
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdbool.h>
#include "uart.h"

#define GPS_MAX_FIELD_SIZE 15
#define SIZE_OF_COORDINATE 40

class GPS
{

private:
    // UART communication
    UART *serialGPS;

    // parsing state variables
    char term[GPS_MAX_FIELD_SIZE];
    uint8_t parity;
    bool isChecksumTerm;
    bool sentenceTypeGNGLL;
    uint8_t curTermNumber;
    uint8_t curTermOffset;
    uint8_t isDataValid;

    // position 
	char JSON_Coordinate[SIZE_OF_COORDINATE];

    // statistics
    uint32_t validFixCount;
    uint32_t invalidFixCount;
    uint32_t failedChecksumCount;
    uint32_t passedChecksumCount;

public:
    GPS(UART *serialInput);
    bool isConnected();
    void init(uint32_t baud);
    char* getPosition(void);

private:
	// parsing functions
    bool encode(char c); // process one character received from GPS
    void setStatus(const char *term);

    // internal utilities
    int HextoDec(char a);
    bool termHandler();
};

#endif /* GPS_H_ */

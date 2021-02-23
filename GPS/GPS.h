/*
 * GPS.h
 *
 * Created: 17/02/2021 18:45:19
 *  Author: Karim Bouanane
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>

#define _GPS_MAX_FIELD_SIZE 15

struct UTC
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t centisecond;

    UTC() : hour(0), minute(0), second(0), centisecond(0) {}
};

class GPS
{
public:
    //GPS();

    bool encode(char c); // process one character received from GPS

    bool locationisValid() { return locationValid; };
    double getLatitude() { return latitude; };
    double getLongitude() { return longitude; };

    uint8_t getHour() { return time.hour; };
    uint8_t getMinutes() { return time.minute; };
    uint8_t getSecond() { return time.second; };
    uint16_t getCentisecond() { return time.centisecond; };

private:
    enum
    {
        GPS_SENTENCE_GNGLL,
        GPS_SENTENCE_OTHER
    };

    //
    void calculateLatitude(char *term);
    void calculateLongitude(char *term);
    void setSIndicator(bool);
    void setWIndicator(bool);
    void setTime(char *term);
    void setStatus(const char *term);

    double latitude, longitude;
    UTC time;

    // parsing state variables
    char term[_GPS_MAX_FIELD_SIZE];
    uint8_t parity;
    bool isChecksumTerm;
    bool locationValid;
    uint8_t curSentenceType;
    uint8_t curTermNumber;
    uint8_t curTermOffset;
    uint8_t isStatusValid;

    // internal utilities
    int HextoDec(char a);
    bool termHandler();
};

#endif /* GPS_H_ */

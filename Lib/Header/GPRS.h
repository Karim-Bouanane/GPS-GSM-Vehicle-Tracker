/*
 * GPRS.h
 *
 * 
 * Author: Karim Bouanane
 */ 


#ifndef GPRS_H_
#define GPRS_H_

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include "swuart.h"

#define MAX_DELAY	0xFFFFFFFF
#define MAX_RETRY	0xFF


typedef enum
{
	GPRS_TIMEOUT_REACHED	= 0,
	GPRS_SUCCESS_REPLY,
	GPRS_ERROR_REPLY,
	UNINDENTIFIED_ERROR,
		
	/**** MODULE ****/

	// Module Connection
	GPRS_DISCONNECTED,
	RESTART_FAIL,
	
	// SIM Card
	NO_SIM_CARD,
	PIN_REQUIRED,
	
	/**** GPRS ****/
	
	// GPRS Network
	GPRS_REGISTRATION_FAIL,
	ATTACH_NETWORK_FAIL,
	
	// PDP Context
	ACTIVATE_PDPCONTEXT_FAIL,
	DEACTIVATE_PDPCONTEXT_FAIL,

	// HTTP Request
	HTTP_SENDING_ERROR,
	HTTP_CLIENT_ERROR,
	HTTP_SERVER_ERRORS,
	HTTP_UNKNOWN_ERROR,
	
	/**** GSM ****/
	
	// GSM registration
	GSM_REGISTERATION_FAIL,
		
	// SMS
	SMS_SENDING_ERROR,
	
	// Balance
	CHECK_BALANCE_ERROR,
	
}GPRSCode;


typedef enum
{
	Mode_normal = 0,
	Mode_gpio_power_down = 1,
	Mode_serial_power_down = 2
	
}SleepMode;


class GPRS
{

	private: // private variables
	
		uint16_t errorCode;
		
	public : // public methods
		
		/**** A9 module ****/
		void initSerial();
		uint8_t waitReady();
		uint8_t isConnected();
		uint8_t restart();
		uint8_t softReset();
		uint8_t sleep(SleepMode type);
		
		// AT Command
		uint8_t sendAT(const char* ATCommand, const char* exptReply, uint32_t timeout, uint8_t retry, bool catchError = false);
		uint8_t waitResponse(const char* exptReply, uint32_t timeout, bool catchError = false);
		uint16_t getErrorCode();
		void setErrorCode(uint16_t code);
				
		// SIM Card
		uint8_t isPINUnlocked();
		uint8_t isSIMInserted();
		
		/**** GPRS ****/
		uint8_t waitGPRSReg();
		uint8_t attachMT();
		uint8_t setupPDPContext();
		uint8_t unsetupPDPContext();
		uint8_t configureAPN(const char* apn);
		uint8_t activateGPRS(const char* apn);
		
		// HTTP
		uint8_t send_HTTP_POSTRequest(const char* httpURL, const char* contentType, const char* postData, uint8_t retry=1);
		
		/**** GSM ****/
		uint8_t waitGSMReg();
		uint8_t setSMSTextFormat();
		uint8_t sendSMS(const char* phone_number, const char* message);
		uint8_t readSMS(const char* phone_number ,const char* recv_message, size_t len);
		
		// Balance
		uint8_t checkBalance(const char* code);
		
		// Location
		uint8_t getLocation();

};

#endif /* GPRS_H_ */
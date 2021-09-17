/*
 * GPRS.h
 *
 * Author: Karim Bouanane
 */ 

#include "GPRS.h"

SWUART serialGPRS;


// Debugging function

inline void DebugPulse(uint8_t count)
{
	while (count--)
	{
		PORTD |= _BV(7);
		PORTD &= ~_BV(7);
	}
}


/**** A9 module ****/

void GPRS::initSerial()
{
	serialGPRS.init(9600);
	
	// I already configured manually some parameters and stored them in a defined profile.
	// This profile is automatically restored after power-up, here they are command that :
	// I used:
	// AT+IPR = 9600		Change baud rate
	// AT+CMGF = 1			SMS Message in text format
}


uint8_t GPRS::softReset()
{
	// SUCCESS		OK
	//
	// ERROR		NONE
	//				
		
	uint8_t status;
	
	status = sendAT("AT+RST=1", "OK\r\n", 2000, 2);	// send command and catch error if there is one 
	
	if (status == GPRS_SUCCESS_REPLY)
	{
		waitReady();					// wait for the module till it finishes the restart
	}
	
	return status;	
}


uint8_t GPRS::restart()
{
	// SUCCESS		OK
	//
	// ERROR		NONE
	//				
		
	uint8_t status;
	
	status = sendAT("AT+RST=2", "OK\r\n", 2000, 2);
	
	if (status == GPRS_SUCCESS_REPLY)
	{
		waitReady();					// wait for the module till it finishes the restart
	}
	
	return status;
}


uint8_t GPRS::waitReady()
{
	uint8_t status;
	
	status = waitResponse("READY", 45000);	
	
	if(status != GPRS_SUCCESS_REPLY)					
	{
		if(isSIMInserted() != GPRS_SUCCESS_REPLY)
		{
			status = NO_SIM_CARD;
		}
		else if(isPINUnlocked() != GPRS_SUCCESS_REPLY)
		{
			status = PIN_REQUIRED;
		}
		else if(waitGSMReg() != GPRS_SUCCESS_REPLY)
		{
			status = GSM_REGISTERATION_FAIL;
		}
		else
			status = GPRS_SUCCESS_REPLY;
	}
		
	_delay_ms(100);			// delay between the sending of commands
	return status;
}


uint8_t GPRS::isConnected()
{
	// SUCCESS		OK
	//
	// ERROR		NONE
	//				
	
	if(sendAT("AT", "OK\r\n", 2000, 3) != GPRS_SUCCESS_REPLY)
		return GPRS_DISCONNECTED;
	
	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::sleep(SleepMode type)
{
	switch(type)
	{
		case Mode_normal:
			return sendAT("AT+SLEEP=0", "OK\r\n", 2000, 2);
		break;
		
		case Mode_gpio_power_down:
			return sendAT("AT+SLEEP=1", "OK\r\n", 2000, 2);
		break;
		
		case Mode_serial_power_down:
			return sendAT("AT+SLEEP=2", "OK\r\n", 2000, 2);
		break;
	}
	
	return 0;
}


/**** AT Command ****/

uint8_t GPRS::sendAT(const char* ATCommand, const char* exptReply, uint32_t timeout, uint8_t retry, bool catchError)
{
	uint8_t status;
	
	while (retry--)	// retry sending command till we get exptReply
	{
		serialGPRS.sendString(ATCommand);	// send AT command
		serialGPRS.sendString("\r\n");		// send command terminator
		
		status = waitResponse(exptReply, timeout, catchError);
		
		if (status == GPRS_SUCCESS_REPLY || status == GPRS_ERROR_REPLY)
		{
			_delay_ms(100); // delay between the sending of commands
			return status;	
		}
	}
	
	return GPRS_TIMEOUT_REACHED;
}


uint8_t GPRS::waitResponse(const char* exptReply, uint32_t timeout, bool catchError)
{
	char strcode[5];	// store the code of error
	uint8_t status;
	
	if(catchError)
	{
		// findOneOf return 0 if timeout is reached, or 1 if the first message is found, or 2 if the second message is found
		status = serialGPRS.findOneOf(exptReply, "+CME ERROR", timeout);
		
		if(status == GPRS_ERROR_REPLY)					 
		{
			serialGPRS.readString(strcode, 5, 50);	// get the error code
			setErrorCode(atoi(strcode));			// convert strcode to integer and 
		}
	}
	else
	{
		// find return 0 if timeout is reached, or 1 if exptReply is found
		status = serialGPRS.find(exptReply, timeout);
	}
	
	return status;
}


uint16_t GPRS::getErrorCode()
{
	return errorCode;
}


void GPRS::setErrorCode(uint16_t code)
{
	errorCode = code;
}


/**** SIM Card ****/

uint8_t GPRS::isSIMInserted()
{
	// SUCCESS		OK
	//	
	// ERROR		NONE
	//
	
	if(sendAT("AT+CCID", "+CCID:", 2000, 2) != GPRS_SUCCESS_REPLY)
		return NO_SIM_CARD;

	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::isPINUnlocked()
{
	// SUCCESS		+CPIN: <code>
	//				code = READY, SIM PIN, SIM PUK, SIM PIN2, SIM PUK2 
	//
	// ERROR		NONE
	//
	
	if(sendAT("AT+CPIN?", "+CPIN:READY", 2000, 2) != GPRS_SUCCESS_REPLY)
		return PIN_REQUIRED;
	
	return GPRS_SUCCESS_REPLY;
}


/**** GPRS ****/

uint8_t GPRS::waitGPRSReg()
{
	// SUCCESS		+CGREG: n,stat
	//				n= 0: disable unsolicited result code. 1: enable unsolicited result code. 
	//				   2: enable location information 
	//				stat= 0: not registered and not searching. 1: registered home network. 
	//					  2: not registered and currently searching. 3: registration denied
	//					  4: unknown. 5: registered roaming 	
	//				
	// ERROR		NONE
	//

	uint8_t status;

	sendAT("AT+CGREG=0", "OK\r\n", 1000, 2);	// disable unsolicited result
	
	status = sendAT("AT+CGREG?", "+CGREG: 0,1", 1000, MAX_RETRY);

	if (status != GPRS_SUCCESS_REPLY)
		return GPRS_REGISTRATION_FAIL;

	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::setupPDPContext()
{
	// SUCCESS		OK
	//
	// ERROR		+CME ERROR: 50							+CME ERROR: 148
	//				Requested facility not subscribed		Unspecified GPRS error
	
	if(sendAT("AT+CGACT=1,1", "OK\r\n", 45000, 5, true) != GPRS_SUCCESS_REPLY)
		return ACTIVATE_PDPCONTEXT_FAIL;
		
	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::unsetupPDPContext()
{
	// SUCCESS		OK
	//
	// ERROR		NONE
	//
	
	if(sendAT("AT+CGACT=0,1", "OK\r\n", 5000, 3, true) != GPRS_SUCCESS_REPLY)
		return DEACTIVATE_PDPCONTEXT_FAIL;
	
	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::attachMT()
{
	// SUCCESS		OK
	//
	// ERROR		COMMAND NO RESPONSE
	//				in case MT not registered (+CGREG: 0,0)
	
	if(sendAT("AT+CGATT=1", "+CGATT:1", 45000, 5, true) != GPRS_SUCCESS_REPLY)
		return ATTACH_NETWORK_FAIL;
	
	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::configureAPN(const char *apn)
{
	// SUCCESS		OK
	//
	// ERROR		+CME ERROR: 53
	//				Parameters invalid
	
	if(unsetupPDPContext() != GPRS_SUCCESS_REPLY)	// Deactivate PDP context, so as to configure it again
		return DEACTIVATE_PDPCONTEXT_FAIL;
	
	serialGPRS.sendString("AT+CGDCONT=1,\"IP\",\"");
	serialGPRS.sendString(apn);
	serialGPRS.sendString("\"\r\n");
	
	waitResponse("OK\r\n", 3000);
	
	_delay_ms(100);
	return GPRS_SUCCESS_REPLY;
}
	
	
uint8_t GPRS::activateGPRS(const char* apn)
{
	if(waitGSMReg() != GPRS_SUCCESS_REPLY)
		return GSM_REGISTERATION_FAIL;
			
	// wait for gprs registration
	if(waitGPRSReg() != GPRS_SUCCESS_REPLY)
		return GPRS_REGISTRATION_FAIL;
	
	// subscribe to network	
	if(attachMT() != GPRS_SUCCESS_REPLY)
		return ATTACH_NETWORK_FAIL;
		
	// configure pdp context
	if (configureAPN(apn) != GPRS_SUCCESS_REPLY)
		return DEACTIVATE_PDPCONTEXT_FAIL;
	
	// activate pdp context
	if (setupPDPContext() != GPRS_SUCCESS_REPLY)
		return ACTIVATE_PDPCONTEXT_FAIL;
	
	return GPRS_SUCCESS_REPLY;
}


/**** HTTP ****/

uint8_t GPRS::send_HTTP_POSTRequest(const char* httpURL, const char* contentType, const char* postData, uint8_t retry)
{
	// Format of command: AT+HTTPPOST = <url>, <content_type>, <body_content>

	// SUCCESS		OK
	//
	// ERROR		+CME ERROR: 53
	//				Sim card or Dns fail
	
	uint8_t status = HTTP_SENDING_ERROR;
	uint8_t found;
	char codeStr[4];
	uint16_t codeInt;
	char temp;
		
	while(retry--) 
	{
		DebugPulse(1);
		
		serialGPRS.sendString("AT+HTTPPOST= \"");
		serialGPRS.sendString(httpURL);
		serialGPRS.sendString("\" , \"");
		serialGPRS.sendString(contentType);
		serialGPRS.sendString("\" , \"");
		serialGPRS.sendString(postData);
		serialGPRS.sendString("\" \r\n");
	
		found = serialGPRS.findOneOf("HTTP/1.1  ", "+CME ERROR", 120000);
		DebugPulse(2);
		
		if(found == 1)
		{	
			serialGPRS.readString(codeStr, 4,  50);	
			codeInt = atoi(codeStr);
			
			while(serialGPRS.read(&temp, 50) != false);	// read the whole response
			
			if(codeInt >= 200 && codeInt <= 299)
			{
				status = GPRS_SUCCESS_REPLY;		
				break;
			}
			
			else if(codeInt >= 400 && codeInt <= 499)
			{
				status = HTTP_CLIENT_ERROR;
			}
			
			else if(codeInt >= 500 && codeInt <= 599)
			{
				status = HTTP_SERVER_ERRORS;
			}
			
			else
			{
				status = HTTP_UNKNOWN_ERROR;
			}


		}
		else
		{
			status = HTTP_SENDING_ERROR;
			break;
		}
	
		_delay_ms(100);
	
	}
	
	_delay_ms(100);
	return status;
}


/**** GSM ****/

uint8_t GPRS::waitGSMReg()
{
	// SUCCESS		+CREG: n,stat
	//				n= 0: disable unsolicited result code. 1: enable unsolicited result code.
	//				   2: enable location information
	//				stat= 0: not registered and not searching. 1: registered home network.
	//					  2: not registered and currently searching. 3: registration denied
	//					  4: unknown. 5: registered roaming
	//
	// ERROR		NONE
	//

	uint8_t status;
	//char code;
	
	sendAT("AT+CREG=0","OK\r\n",1000,2);	// Disable network registration unsolicited result
	
	status = sendAT("AT+CREG?", "+CREG: 0,1", 2000, MAX_RETRY);
	
	if (status != GPRS_SUCCESS_REPLY)
		return GSM_REGISTERATION_FAIL;
	
	return GPRS_SUCCESS_REPLY;
}


uint8_t GPRS::setSMSTextFormat()
{
	// SUCCESS		OK
	//
	// ERROR		NONE
	//
	
	return sendAT("AT+CMGF=1", "OK\r\n", 2000, 2);
}


uint8_t GPRS::sendSMS(const char* phone_number, const char* message)
{
	// SUCCESS		+CMGS: 1						+CMGS: 2				+CMGS: 3
	//				phone number isn't allowed		successful sending		
	//
	// ERROR		+CMS ERROR						COMMAND NO RESPONSE
	//				failing to send message			if 0x1A is ignored
	
	uint8_t status;

	serialGPRS.sendString("AT+CMGS=");
	serialGPRS.send('\"');
	serialGPRS.sendString(phone_number);
	serialGPRS.sendString("\"\r\n");
	
	waitResponse(">", 1000);
	_delay_ms(50);
	
	serialGPRS.sendString(message);
	serialGPRS.send(0x1A);
	
	status = serialGPRS.findOneOf("+CMS ERROR", "+CMGS:", 45000);
	
	if(status != 2)
		status = SMS_SENDING_ERROR;
	else
		status = GPRS_SUCCESS_REPLY;
	
	_delay_ms(100);
	return status;
}


/**** Balance ****/

uint8_t GPRS::checkBalance(const char* code)
{
	// iam code #580#
	// orange code #554#
		
	// SUCCESS		OK
	//
	// ERROR		+CME ERROR: 15790320 			+CUSD: 2
	//												session closed
	
	uint8_t status;

	serialGPRS.sendString("AT+CUSD=1,\"");
	serialGPRS.sendString(code);
	serialGPRS.sendString("\",15\r\n");
	
	status = serialGPRS.findOneOf("+CUSD: 1", "+CUSD: 2", 6000);
	
	if(status != 1)
		status = CHECK_BALANCE_ERROR;
	else
		status = GPRS_SUCCESS_REPLY;
	
	_delay_ms(500);
	return status;
}


/**** Location ****/


#include "WiFi.h"
#include "lcd_driver.h"
#include "cmsis_os.h"
#include "Serial.h"
#include "String.h"

#define Delay osDelay

// Needed for sprintf?
#include "stm32f4xx_hal_gpio.h"

int handler = 0;
uint8_t WiFiRXString[17];
int WiFiATResponse = 0;
int WiFiRXIndex = 0;
int WiFiATCheckMode = 0;

void RXHandler (uint8_t ch);

int WiFiRXState = 0;
int WiFiStringPos = 0;
char WiFiRXOut[17];
int WiFiModeInt = 1;

void WiFiSendString(char* string) {
	Serial_Send((uint8_t*)string, strlen(string));
}


int Check_For_WiFi() {
	int i;
	int ret = 0;
	Register_RX_Handler(&RXHandler);
	WiFiATCheckMode = 1;
	for (i=0; i<3; i++) { 
		WiFiSendString("AT+RST\r\n");
		Delay(100);
		//check response
		if (WiFiATResponse == 1) {
			ret = 1;
			break;
		}
	}
	Deregister_RX_Handler();
	WiFiATCheckMode = 0;
	return ret;
}


void WiFi_Init() {
	Register_RX_Handler(&RXHandler);
	WiFiATCheckMode = 1;

	int ret = 0;
	WiFiATResponse = 0;
	
	if (WiFiEnabled == 1) {
		
		// Set mode (1=station, 2=access point, 3=both)
		WiFiSendString("AT+CWMODE=2\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set SSID, password, channel, encoding
		// encoding: 0=open, 2=WPA_PSK, 3=WPA2_PSK, 4=WPA_WPA2_PSK
		WiFiSendString("AT+CWSAP=\"MultiMeter\",\"123\",1,0\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set single (0) or multiple (1) connection mode
		WiFiSendString("AT+CIPMUX=1\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Enable or disable DHCP server
		// <mode>,<enable>
		// mode: 0=software aceess point, 1=station, 2=both
		// enable: 0=enabled, 1=disabled
		WiFiSendString("AT+CWDHCP=0,0\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set access point IP address
		WiFiSendString("AT+CIPAP=\"192.168.1.1\"\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set server IP address and port
		WiFiSendString("AT+CIPSERVER=1,1138\r\n");
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
	}
	
	//return(ret);
	
	//Deregister_RX_Handler();
	WiFiATCheckMode = 0;
	WiFiRXState = 4;
}


void WiFi_Send(uint8_t *pData, uint16_t Size) {	
	char string[17];
	sprintf(string, "AT+CIPSEND=0,%d\r\n", Size);
	Serial_Send((uint8_t*)string, strlen(string));
	Serial_Send(pData, Size);
	
	// Uncomment for second connection support
	/*
	sprintf(string, "AT+CIPSEND=1,%d\r\n", Size);
	Serial_Send((uint8_t*)string, strlen(string));
	Serial_Send(pData, Size);
	*/
	
	if (WiFiATResponse == 1) {
		lcd_clear_display();
		lcd_write_string("AT OK", 0, 0);
		Delay(1000);
		lcd_clear_display();
	}
	if (WiFiATResponse == 2) {
		lcd_clear_display();
		lcd_write_string("AT ERROR", 0, 0);
		Delay(1000);
		lcd_clear_display();
	}
}


void WiFi_Receive() {
	if (strcmp(WiFiRXOut, "") != 0) {
		char string[17];
		sprintf(string, "Serial:");
		lcd_clear_display();
		lcd_write_string(string, 0, 0);
		sprintf(string, "%s", WiFiRXOut);
		lcd_write_string(string, 1, 0);
		Delay(5000);
		lcd_clear_display();
		sprintf(WiFiRXOut ,"");
	}
}


void WiFi_Check_Mode(int *mode) {
	if (WiFiModeInt != 10) {
		*mode = WiFiModeInt;
		WiFiModeInt = 10;
	}
}












void WiFi_AT_Check (uint8_t byte) {
	int i = 0;
	if (byte == '\r') {  // Ignore /r
	
	}
	else if (byte == '\n') { // If new line
		WiFiRXString[WiFiRXIndex] = 0;
		if (strcmp((char*)WiFiRXString, "OK") == 0) {
			WiFiATResponse = 1;
		}
		if (strcmp((char*)WiFiRXString, "ERROR") == 0) {
			WiFiATResponse = 2;
		}
		WiFiRXIndex = 0;
		for (i = 0; i < 17; i++) WiFiRXString[i] = 0; // Clear the string buffer
	}

	else
	{
			WiFiRXString[WiFiRXIndex] = byte; // Add that character to the string
			WiFiRXIndex++;
			if (WiFiRXIndex >= 17) // User typing too much, we can't have commands that big
			{
					WiFiRXIndex = 0;
					for (i = 0; i < 17; i++) WiFiRXString[i] = 0; // Clear the string buffer
			}
	}
}



















void RXHandler (uint8_t byte) {

	if (WiFiATCheckMode == 1) {
		WiFi_AT_Check (byte);
	}
	else {
		char WiFiString[5];
		sprintf(WiFiString, "+IPD");
		
		if (WiFiRXState == 0) { // Check what to expect
			if (byte == 's') {
				WiFiRXState = 1;
			}
			if (byte == 'm') {
				WiFiRXState = 2;
			} 
		}
		
		else if (WiFiRXState == 1) { // expect string
		
			int i = 0;

			if (byte == '\n' || byte == '\r') // If Enter
			{
				WiFiRXString[WiFiRXIndex] = 0;
				sprintf(WiFiRXOut, "%s", WiFiRXString);
				WiFiRXIndex = 0;
				for (i = 0; i < 17; i++) WiFiRXString[i] = 0; // Clear the string buffer
				
				// String complete, go back to check
				if (WiFiEnabled == 1)
					WiFiRXState = 4;
				else
					WiFiRXState = 0;
			}

			else
			{
					WiFiRXString[WiFiRXIndex] = byte; // Add that character to the string
					WiFiRXIndex++;
					if (WiFiRXIndex >= 17) // User typing too much, we can't have commands that big
					{
							WiFiRXIndex = 0;
							for (i = 0; i < 17; i++) WiFiRXString[i] = 0; // Clear the string buffer
							if (WiFiEnabled == 1)
								WiFiRXState = 4;
							else
								WiFiRXState = 0;
							sprintf(WiFiRXOut, "String too long");
					}
			}
		}
		
		else if (WiFiRXState == 2) { // Expect mode
			if (((byte - '0') < 3) & (byte >= '0')){ // valid mode
				WiFiModeInt = (int) byte - '0';
			}
			// RX complete, go back to check
			if (WiFiEnabled == 1)
				WiFiRXState = 4;
			else
				WiFiRXState = 0;
		}
		
		else if (WiFiRXState == 3) { // Check response to AT command
			WiFiRXState = 0;
		}
		
		// Check for "+IPD" start condition from WiFi module
		else if (WiFiRXState == 4) {
			if (WiFiStringPos == 0) {
				if (byte == WiFiString[0]) {
					WiFiStringPos++;
				}
			}
			else if (byte == WiFiString[WiFiStringPos]) {
				WiFiStringPos++;
			}
			else {
				WiFiStringPos = 0;
			}
			
			if (WiFiStringPos >=3) {
				WiFiRXState = 0;
			}
		}
	}
}

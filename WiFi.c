#include "WiFi.h"
#include "lcd_driver.h"
#include "cmsis_os.h"
#include "Serial.h"
#include "String.h"

#define Delay osDelay

// Needed for sprintf?
#include "stm32f4xx_hal_gpio.h"

int handler = 0;
uint8_t WiFiRXString[64];
int WiFiATResponse = 0;
int WiFiRXIndex = 0;

void RX_Handler (uint8_t ch);

char WiFiRXOut[17];
int WiFiModeInt = 1;
int dataQueued = 0;
uint8_t dataQueue[100];
uint16_t queueSize = 0;

int WiFi_Busy = 0;

void WiFi_Send_String(char* string) {
	Serial_Send((uint8_t*)string, strlen(string));
}


void Queue_Data (uint8_t *pData, uint16_t Size) {
	int i;
	for (i=0; i<Size; i++) {
		dataQueue[i] = pData[i];
	}
	queueSize = Size;
	dataQueued = 1;
}

// Blocking function, but only for init
int Check_For_WiFi() {
	int i;
	int ret = 0;
	Register_RX_Handler(&RX_Handler);
	for (i=0; i<3; i++) { 
		WiFi_Send_String("AT+RST\r\n");
		Delay(100);
		//check response
		if (WiFiATResponse == 1) {
			ret = 1;
			break;
		}
	}
	Deregister_RX_Handler();
	return ret;
}

// Blocking function, but only for init
int WiFi_Init() {
	Register_RX_Handler(&RX_Handler);

	int ret = 0;
	WiFiATResponse = 0;
	
	if (WiFiEnabled == 1) {
		
		// Set mode (1=station, 2=access point, 3=both)
		WiFi_Send_String("AT+CWMODE=2\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set SSID, password, channel, encoding
		// encoding: 0=open, 2=WPA_PSK, 3=WPA2_PSK, 4=WPA_WPA2_PSK
		WiFi_Send_String("AT+CWSAP=\"MultiMeter\",\"123\",1,0\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set single (0) or multiple (1) connection mode
		WiFi_Send_String("AT+CIPMUX=1\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Enable or disable DHCP server
		// <mode>,<enable>
		// mode: 0=software aceess point, 1=station, 2=both
		// enable: 0=enabled, 1=disabled
		WiFi_Send_String("AT+CWDHCP=0,0\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set access point IP address
		WiFi_Send_String("AT+CIPAP=\"192.168.1.1\"\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
		
		// Set server IP address and port
		WiFi_Send_String("AT+CIPSERVER=1,1138\r\n");
		while (serial_Busy == 1) {
			Delay(100);
		}
		if (WiFiATResponse != 1)
			ret = 1;
		WiFiATResponse = 0;
	}
	
	sprintf(WiFiRXOut ,"");
	
	//Deregister_RX_Handler();
	return(ret);
}


void WiFi_Send(uint8_t *pData, uint16_t Size) {	
	if ((WiFi_Busy == 0) && (serial_Busy == 0)){
		WiFiATResponse = 0;
		//Serial_StatusTypeDef ret;
		WiFi_Busy = 1;
		char string[17];
		Queue_Data(pData, Size);
		sprintf(string, "AT+CIPSEND=0,%d\r\n", Size);
		Serial_Send((uint8_t*)string, strlen(string));
	}
}


void WiFi_Receive() {
	if (strcmp(WiFiRXOut, "") != 0) {
		char string[17];
		sprintf(string, "WiFi:");
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













void Check_String () {
	
	if (strstr((char*)WiFiRXString, "+IPD") != NULL) {
		// receive string from WiFi
		if (WiFiRXString[9] == 'm') {
			// handle mode change
			uint8_t byte = WiFiRXString[10];
			if (((byte - '0') < 6) & (byte >= '0')){ // valid mode
				WiFiModeInt = (int) byte - '0';
			}
		}
		else if (WiFiRXString[9] == 's') {
			// handle string
			sprintf(WiFiRXOut, "%s", WiFiRXString+10);
		}
		else {
			sprintf(WiFiRXOut, "Undefined ID %c", WiFiRXString[10]);
		}

	}
	
	else if ((strstr((char*)WiFiRXString, "ERROR") != NULL) || (strstr((char*)WiFiRXString, "FAIL") != NULL)) {
		WiFiATResponse = 2;
		dataQueued = 0;
		WiFi_Busy = 0;
	}
	
	else if (strstr((char*)WiFiRXString, "OK") != NULL) {
		WiFiATResponse = 1;
		if (dataQueued == 1) {
			WiFi_Busy = 1;
			dataQueued = 0;
			Serial_StatusTypeDef ret;
			ret = Serial_Send(dataQueue, queueSize);
			if (ret != SERIAL_OK) {
				WiFi_Busy = 0;
			}
			else {
				WiFi_Busy = 1;
			}
		}
		else {
			WiFi_Busy = 0;
		}
	}
}



void RX_Handler (uint8_t byte) {
	
	// Receive string
	if (byte == '\n' || byte == '\r') { // If Enter
		WiFiRXString[WiFiRXIndex] = 0;
		Check_String();
		WiFiRXIndex = 0;
		int i;
		for (i = 0; i < sizeof(WiFiRXString); i++) WiFiRXString[i] = 0; // Clear the string buffer
	}
	else { // build string
		WiFiRXString[WiFiRXIndex] = byte; // Add that character to the string
		WiFiRXIndex++;
		if (WiFiRXIndex >= sizeof(WiFiRXString)) // User typing too much, we can't have commands that big
		{
				WiFiRXIndex = 0;
				int i;
				for (i = 0; i < sizeof(WiFiRXString); i++) WiFiRXString[i] = 0; // Clear the string buffer
				sprintf(WiFiRXOut, "String too long");
		}
	}
}


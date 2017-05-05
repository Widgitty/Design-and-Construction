#include "WiFi.h"
#include "lcd_driver.h"
#include "cmsis_os.h"
#include "Serial.h"
#include "String.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"

#define Delay osDelay


int handler = 0;
uint8_t WiFiRXString[64];
int WiFiATResponse = 0;
int WiFiRXIndex = 0;

char WiFiRXOut[17];
int WiFi_Mode_Int = -1;
int dataQueued = 0;
uint8_t dataQueue[100];
uint16_t queueSize = 0;
int WiFi_CID = 0;

int WiFi_Busy = 0;

void RX_Handler (uint8_t ch);
int WiFi_Send_String(char* string);
void Queue_Data (uint8_t *pData, uint16_t Size);








//=====================================================//
//================ Interface functions ================//
//=====================================================//


//==================================================//
//================= Hardware reset =================//
//==================================================//
// 																									//
//==================================================//

void Reset_WiFi() {

	// Enable GPIO clock
	__HAL_RCC_GPIOC_CLK_ENABLE();	
	
	// Initialise GPIOs
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_1;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// Register handler to avoid Serial.c hadling the reset message
	Register_RX_Handler(&RX_Handler);
	// Togle reset pin
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	Delay(100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	Delay(100);
	Deregister_RX_Handler();
}


// Blocking function, but only for init
int Check_For_WiFi() {
	int i;
	int ret = 0;
	WiFiATResponse = 0;
	Register_RX_Handler(&RX_Handler);
	// Attempt 3 times
	for (i=0; i<3; i++) { 
		// Send AT command to reset module
		if (WiFi_Send_String("AT+RST\r\n") == 1) {
			ret = 1;
			break;
		}
	}
	Deregister_RX_Handler();
	return ret;
}


// Blocking function, but only for init
// This function sends a series of AT commands to 
// initialise the WiFi module.
int WiFi_Init() {
	Register_RX_Handler(&RX_Handler);

	int ret = 0;
	WiFiATResponse = 0;
	
	// Set mode (1=station, 2=access point, 3=both)
	ret = WiFi_Send_String("AT+CWMODE=2\r\n");
	
	// Set SSID, password, channel, encoding
	// encoding: 0=open, 2=WPA_PSK, 3=WPA2_PSK, 4=WPA_WPA2_PSK
	ret = WiFi_Send_String("AT+CWSAP=\"MultiMeter\",\"123\",1,0\r\n");
	
	// Set single (0) or multiple (1) connection mode
	ret = WiFi_Send_String("AT+CIPMUX=1\r\n");
	
	// Enable or disable DHCP server
	// <mode>,<enable>
	// mode: 0=software aceess point, 1=station, 2=both
	// enable: 0=enabled, 1=disabled
	ret = WiFi_Send_String("AT+CWDHCP=0,0\r\n");
	
	// Set access point IP address
	ret = WiFi_Send_String("AT+CIPAP=\"192.168.1.1\"\r\n");
	
	// Set server IP address and port
	ret = WiFi_Send_String("AT+CIPSERVER=1,1138\r\n");
	
	sprintf(WiFiRXOut ,"");
	
	return(ret);
}


// This function queues the data, then sends an AT 
// command to initiate a data transmit. the data is
// then send when an "OK" response is received (from
// the RX_Handler).
void WiFi_Send(uint8_t *pData, uint16_t Size) {	
	if ((WiFi_Busy == 0) && (serial_Busy == 0)){
		WiFiATResponse = 0;
		//Serial_StatusTypeDef ret;
		WiFi_Busy = 1;
		char string[17];
		Queue_Data(pData, Size);
		sprintf(string, "AT+CIPSEND=%d,%d\r\n",WiFi_CID, Size);
		WiFi_CID ++;
		if (WiFi_CID >= 3) {
			WiFi_CID = 0;
		}
		Serial_Send((uint8_t*)string, strlen(string));
	}
}


// This function just prints any recieved string
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


// This function returns the requested mode
void WiFi_Check_Mode(int *mode) {
	if (WiFi_Mode_Int != -1) {
		*mode = WiFi_Mode_Int;
		WiFi_Mode_Int = -1;
	}
}








//=====================================================//
//================ Supporting functions ===============//
//=====================================================//



int WiFi_Send_String(char* string) {
	int ret = 0;
	Serial_Send((uint8_t*)string, strlen(string));
	while (serial_Busy == 1) {
		Delay(100);
	}
	if (WiFiATResponse != 1)
		ret = 1;
	WiFiATResponse = 0;
	return ret;
}


// Add data to the queue (more accurately, store an
// array to send later)
void Queue_Data (uint8_t *pData, uint16_t Size) {
	int i;
	for (i=0; i<Size; i++) {
		dataQueue[i] = pData[i];
	}
	queueSize = Size;
	dataQueued = 1;
}



void Check_String () {
	if (strstr((char*)WiFiRXString, "+IPD") != NULL) {
		// receive string from WiFi
		if (WiFiRXString[9] == 'm') {
			// handle mode change
			uint8_t byte = WiFiRXString[10];
			if (((byte - '0') < 9) & (byte >= '0')){ // valid mode
				WiFi_Mode_Int = (int) byte - '0';
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
		// send queued data if there is any
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
	if (byte == '\n' || byte == '\r') { // If new line
		WiFiRXString[WiFiRXIndex] = 0;
		// handle whatever was received
		Check_String();
		WiFiRXIndex = 0;
		int i;
		for (i = 0; i < sizeof(WiFiRXString); i++) WiFiRXString[i] = 0; // Clear the string buffer
	}
	else { // build string
		WiFiRXString[WiFiRXIndex] = byte; // Add character to the string
		WiFiRXIndex++;
		if (WiFiRXIndex >= sizeof(WiFiRXString)) // Prevent overflow
		{
				WiFiRXIndex = 0;
				int i;
				for (i = 0; i < sizeof(WiFiRXString); i++) WiFiRXString[i] = 0; // Clear string buffer
				sprintf(WiFiRXOut, "String too long");
		}
	}
}


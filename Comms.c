#include "Comms.h"
#include "WiFi.h"
#include "Serial.h"
#include "lcd_driver.h"
#include "cmsis_os.h"

#define Delay osDelay

int WiFi_Present = 0;
int serialEnabled = 0;
int WiFiEnabled = 0;

void Comms_Init() {
	serialEnabled = Check_For_Serial();
	if (serialEnabled == 1) {
		SerialInit();
		SerialReceiveStart();
		WiFiEnabled = Check_For_WiFi();
		if (WiFiEnabled == 1) {
			WiFi_Init();
		}

		// Debug, print selected mode
		if(WiFiEnabled == 1) {
			lcd_clear_display();
			lcd_write_string("WiFi Mode", 0, 0);
			Delay(1000);
			lcd_clear_display();
		}
		else {
			lcd_clear_display();
			lcd_write_string("Serial Mode", 0, 0);
			Delay(1000);
			lcd_clear_display();
		}
	}
	else {
		lcd_clear_display();
		lcd_write_string("Comms Disabled", 0, 0);
		Delay(1000);
		lcd_clear_display();
	}
}


void Comms_Send(double value, uint8_t mode, uint8_t range) {
	uint8_t data[17] = {0};
	uint8_t *datap;
	uint8_t START_BYTE = 'D';
	uint8_t checksum = 0x00;
	datap = (uint8_t*) &value;
	
	// Serialise data
	
	// Build data packet
	// start byte
	data[0] = START_BYTE;
	checksum = checksum ^ data[0];
	// datatype (only '1' currently supported)
	data[1] = '1';
	checksum = checksum ^ data[1];
	// data
	data[2] = *(datap+7);
	checksum = checksum ^ data[2];
	data[3] = *(datap+6);
	checksum = checksum ^ data[3];
	data[4] = *(datap+5);
	checksum = checksum ^ data[4];
	data[5] = *(datap+4);
	checksum = checksum ^ data[5];
	data[6] = *(datap+3);
	checksum = checksum ^ data[6];
	data[7] = *(datap+2);
	checksum = checksum ^ data[7];
	data[8] = *(datap+1);
	checksum = checksum ^ data[8];
	data[9] = *(datap+0);
	checksum = checksum ^ data[9];
	// range
	data[10] = range;
	checksum = checksum ^ data[10];
	// mode
	data[11] = mode;
	checksum = checksum ^ data[11];
	// checksum
	data[12] = checksum;

	if(WiFiEnabled == 1) {
		WiFi_Send((uint8_t*)data, 13);
	}
	else {
		Serial_Send((uint8_t*)data, 13);
	}
}


void Comms_Receive(void) {
	if(WiFiEnabled == 1) {
		WiFi_Receive();
	}
	else {
		Serial_Receive();
	}
}


void Comms_Check_Mode(int *mode) {
	if(WiFiEnabled == 1) {
		WiFi_Check_Mode(mode);
	}
	else {
		Serial_Check_Mode(mode);
	}
}

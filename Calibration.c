#include "Calibration.h"
#include "STM32F4xx_hal.h"
#include "cmsis_os.h"
#include "LCD_Thread.h"
#include "SWT.h"
#include "Math.h"
#include "ADC.h"
#include "Calculations.h" // should be able to remove this later? or integrate the files together

#define Delay osDelay

// File specific variables
I2C_HandleTypeDef hi2c1;

calibStructTypeDef calibStruct;
calibStoreTypeDef momoryStruct;


void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
		GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_I2C3_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	// I2C1 GPIO Configuration
	// PA8     ------> I2C3_SCL
	// PC9     ------> I2C3_SDA
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


void I2Cinit(void)
{
	hi2c1.Instance = I2C3;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 56;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
	HAL_I2C_Init(&hi2c1);
}







/*
void Calibrate(int mode, int range) {
	
	uint8_t address = 0x00;
	int cursor = 0;
	int pos = 0;
	char string[17];
	testStructTpeDef testStruct;
	
	
	uint32_t btns = 0;
	LCD_Write_At("", 0, 0, 1);
	
	// Load stucture
	I2Cinit();
	uint8_t data[9];
	// Set address
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, &address, 1, 5000);
	Delay(1);
	// Receive data
	HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<0UL, (uint8_t*)&data, 8, 5000);
	// Convert 'data' array to structure
	testStruct.point1 = *((double*) &data);
	Delay(1);
	// Set address
	address = 0x08;
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, &address, 1, 5000);
	Delay(1);
	// Receive data
	HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<0UL, (uint8_t*)&data, 8, 5000);
	// Convert 'data' array to structure
	testStruct.point2 = *((double*) &data);

	double number;
	
	switch (mode) {
		case 0:
			number = testStruct.point1;
			address = 0x00;
		break;
		case 1:
			number = testStruct.point2;
			address = 0x08;
		break;
		case 2:
			number = 0;
			address = 0x0F;
		break;
		default:
			sprintf(string, "Undefined mode!");
			LCD_Write_At(string, 0, 0, 0);
			Delay(1000);
		break;
	}
	
	
	//number = 1.0;
	
	while ((btns = SWT_Debounce()) != 0x8000) {

		
		switch (btns) {
			case 0x0100:
				if (cursor > 0) {
					cursor --;
					pos --;
					if (cursor == 1)
						cursor --;
				}
			break;
			case 0x0200:
				if (cursor < 4) {
					cursor ++;
					pos ++;
					if (cursor == 1)
						cursor ++;
				}
			break;
			case 0x0400:
				number -= pow(10, (-pos));
			break;
			case 0x0800:
				number += pow(10, (-pos));
			break;
			case 0x1000:
				number = (double) 0.0;
			break;
			default:
				//blah
			break;
		}
		
		sprintf(string, "%1.3lf", number);
		LCD_Write_At(string, 0, 0, 0);
		LCD_Write_At("     ", 0, 1, 0);
		LCD_Write_At("^", cursor, 1, 0); // TODO: Replace with proper cursor
		Delay(100);
	}
	
	// Measure value
	uint32_t value = 0;
	double value_calk = 0;
	value = read_ADC1();
	value = (value *16);
	value_calk = adcConv(mode, value, &range);
	
	// Calculate error
	double error = number - value_calk;
	
	// Correct
	
	// Save
	// Convert structure to 'data' array
	//number = *((double*) &data);
	uint8_t *numpointer = (uint8_t*) &number;
	int i;
	for (i = 0; i<8; i++) {
		data[i+1] = (numpointer[i]);
		//data[i+1] = i;
	}
	data[0] = address;
	
	// Set address and data
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, data, 9, 5000);
	
	
	// Print info
	
	sprintf(string, "Calibrated at:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", number);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);

	sprintf(string, "Adjusted:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", error);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "I2C:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.4lf", number);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	HAL_I2C_DeInit(&hi2c1);
}
*/

calibStoreTypeDef Read_Calibration() {
	calibStoreTypeDef calibStruct;
	uint8_t address = 0x00;
	uint8_t *calibStructPointer = (uint8_t*)&calibStruct;
	int size = sizeof(calibStruct);
	int i = 0;
	
	// Load stucture
	I2Cinit();
	
	// Set address
	for (i=0; i < size; i+=8) {
		address = i;
		HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, &address, 1, 5000);
		Delay(1);
		// Receive data
		HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<0UL, calibStructPointer, 8, 5000);
		// Convert 'data' array to structure
		calibStructPointer += 8;
		Delay(1);
	}
	
	HAL_I2C_DeInit(&hi2c1);
	return calibStruct;
}



calibStructTypeDef Get_Calibration() {
	// Calculate?
	return calibStruct;
}


void Write_Calibration(calibStoreTypeDef calibStruct) {
	uint8_t address = 0x00;
	uint8_t *calibStructPointer = (uint8_t*)&calibStruct;
	int size = sizeof(calibStruct);
	int i, j = 0;
	uint8_t data[9];
	
	// Load stucture
	I2Cinit();
	
	for (i=0; i < size; i+=8) {
		address = i;
		for (j = 0; j<8; j++) {
			data[j+1] = (calibStructPointer[i+j]);
			data[0] = address;
		}
		HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, data, 9, 5000);
		Delay(10);
	}
	
	HAL_I2C_DeInit(&hi2c1);
}


void calibInit() {
	calibStoreTypeDef calibStructWrite;
	
	calibStructWrite.current.lowerPointIn = 1.11;
	calibStructWrite.current.lowerPointOut = 1.12;
	calibStructWrite.current.upperPointIn = 1.21;
	calibStructWrite.current.upperPointOut = 1.22;
	calibStructWrite.resistance.lowerPointIn = 2.11;
	calibStructWrite.resistance.lowerPointOut = 2.12;
	calibStructWrite.resistance.upperPointIn = 2.21;
	calibStructWrite.resistance.upperPointOut = 2.22;
	calibStructWrite.voltage.lowerPointIn = 3.11;
	calibStructWrite.voltage.lowerPointOut = 3.12;
	calibStructWrite.voltage.upperPointIn = 3.21;
	calibStructWrite.voltage.upperPointOut = 3.22;
	
	Write_Calibration(calibStructWrite);
}


void Test_Calibration() {
	char string[17];
	
	calibInit();
	
	calibStoreTypeDef calibStructRead;
	calibStructRead = Read_Calibration();
	
	// Print results
	sprintf(string, "Current low In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.current.lowerPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Current low Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.current.lowerPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Current high In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.current.upperPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Current high Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.current.upperPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Res low In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.resistance.lowerPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Res low Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.resistance.lowerPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Res high In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.resistance.upperPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Res high Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.resistance.upperPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Voltage low In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.voltage.lowerPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Voltage low Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.voltage.lowerPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Voltage high In:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.voltage.upperPointIn);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	sprintf(string, "Voltage high Out:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", calibStructRead.voltage.upperPointOut);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	
	LCD_Write_At("", 0, 0, 1);
	
}











void Calibrate(int mode, int range) {
	char string[17];
	// Load current calibration data
	calibStoreTypeDef calibStruct;
	calibStruct = Read_Calibration();
	
	// Setup variables
	uint32_t value = 0;
	double target_value = 0;
	value = read_ADC1();
	value = (value *16);
	target_value = adcConv(mode, value, &range);
	uint32_t btns = 0;
	int cursor = 0;
	int pos = 0;
	
	LCD_Write_At("", 0, 0, 1);
	
	// Read current value from ADC
	value = read_ADC1();
	value = (value *16);
	target_value = adcConv(mode, value, &range);
	
	
	// Allow user to adjust expected value
	// TODO: fix this interface
	while ((btns = SWT_Debounce()) != 0x8000) {
		switch (btns) {
			case 0x0100:
				if (cursor > 0) {
					cursor --;
					pos --;
					if (cursor == 1)
						cursor --;
				}
			break;
			case 0x0200:
				if (cursor < 4) {
					cursor ++;
					pos ++;
					if (cursor == 1)
						cursor ++;
				}
			break;
			case 0x0400:
				target_value -= pow(10, (-pos));
			break;
			case 0x0800:
				target_value += pow(10, (-pos));
			break;
			case 0x1000:
				target_value = (double) 0.0;
			break;
			case 0x2000:
				calibInit();
				LCD_Write_At("Momory init", 0, 0, 1);
				Delay(2000);
				LCD_Write_At("", 0, 0, 1);
			break;
			default:
				//blah
			break;
		}
		
		sprintf(string, "%1.3lf", target_value);
		LCD_Write_At(string, 0, 0, 0);
		LCD_Write_At("     ", 0, 1, 0);
		LCD_Write_At("^", cursor, 1, 0); // TODO: Replace with proper cursor
		Delay(100);
	}
	
	
	
	
	// Recalculate convertion
	// TODO: based on target_value
	
	
	
	// Write?
	Write_Calibration(calibStruct);
	LCD_Write_At("", 0, 0, 1);
}


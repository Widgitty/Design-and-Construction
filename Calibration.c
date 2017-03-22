#include "Calibration.h"
#include "STM32F4xx_hal.h"
#include "cmsis_os.h"
#include "SWT.h"
#include "Math.h"
#include "ADC.h"
#include "Calculations.h" // should be able to remove this later? or integrate the files together
#include "lcd_driver.h"

#define Delay osDelay

// File specific variables
I2C_HandleTypeDef hi2c1;

calibStructTypeDef calibStruct;
calibStoreTypeDef momoryStruct;

int calibration_flag = 0;



// Test assumes voltage mode with no scaling
double Calib_Conv_Test(int mode, double value, int *rangep) {
	// Convert to input voltage
	double m = calibStruct.voltage.multiplier;
	double c = calibStruct.voltage.zeroOffset;
	// NOTE: this is scaled to 0-1 as an input. this is arbitrary and for testing only.
	double output = ((double)value / (pow(2.0, 16.0)) * m) + c;
	return output;
}





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


void Calculate_Calibration() {
	// y = mx + c
	// output = (input * multiplier) + zero_offset
	
	// (y2-y1) / (x2 - x1) = m
	// c = x1*m + y1
	double x1 = momoryStruct.voltage.lowerPointIn;
	double x2 = momoryStruct.voltage.upperPointIn;
	double y1 = momoryStruct.voltage.lowerPointOut;
	double y2 = momoryStruct.voltage.upperPointOut;
	
	//x1 = 0;
	//x2 = 1;
	//y1 = 0;
	//y2 = 3;
	
	double m = (y2-y1)/(x2-x1);
	double c = (x1*m) + y1;
	//c = 0;
	
	calibStruct.voltage.multiplier = m;
	calibStruct.voltage.zeroOffset = c;
	//momoryStruct;
}


void Calibration_Init() {
	
	momoryStruct.current.lowerPointIn = 1.11;
	momoryStruct.current.lowerPointOut = 1.12;
	momoryStruct.current.upperPointIn = 1.21;
	momoryStruct.current.upperPointOut = 1.22;
	momoryStruct.resistance.lowerPointIn = 2.11;
	momoryStruct.resistance.lowerPointOut = 2.12;
	momoryStruct.resistance.upperPointIn = 2.21;
	momoryStruct.resistance.upperPointOut = 2.22;
	momoryStruct.voltage.lowerPointIn = 0;
	momoryStruct.voltage.lowerPointOut = -6;
	momoryStruct.voltage.upperPointIn = 1;
	momoryStruct.voltage.upperPointOut = 3;
	
	Write_Calibration(momoryStruct);
	Calculate_Calibration();
}


void Test_Calibration() {
	char string[17];
	
	Calibration_Init();
	
	calibStoreTypeDef calibStructRead;
	calibStructRead = Read_Calibration();
	
	// Print results
	lcd_clear_display();
	sprintf(string, "Current low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.current.lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.current.lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.current.upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.current.upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.resistance.lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.resistance.lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.resistance.upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.resistance.upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.voltage.lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.voltage.lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.voltage.upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calibStructRead.voltage.upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	
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
	
	lcd_clear_display();
	
	// Read current value from ADC
	value = read_ADC1();
	value = (value *16);
	//target_value = adcConv(mode, value, &range);
	target_value = Calib_Conv_Test(mode, value, &range);
	
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
				Calibration_Init();
				lcd_write_string("Momory init", 0, 0);
				Delay(2000);
			break;
			default:
				//blah
			break;
		}
		
		sprintf(string, "%1.3lf", target_value);
		lcd_write_string(string, 0, 0);
		lcd_write_string("            ", 1, 0);
		lcd_write_string("^", 1, cursor); // TODO: Replace with proper cursor
		Delay(100);
	}
	
	
	
	
	// Recalculate convertion
	// TODO: based on target_value
	
	
	
	// Write?
	Write_Calibration(calibStruct);
	lcd_clear_display();
}



extern void Set_Calibration_Flag(void) {
	calibration_flag = 1;
}


extern void Check_Calibration_Flag(int mode, int range) {
	if (calibration_flag == 1) {
		calibration_flag = 0;
		Calibrate(mode, range);
		calibration_flag = 0;
	}
}


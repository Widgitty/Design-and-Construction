#include "Calibration.h"
#include "STM32F4xx_hal.h"
#include "cmsis_os.h"
#include "SWT.h"
#include "Math.h"
#include "ADC.h"
#include "Calculations.h" // should be able to remove this later? or integrate the files together
#include "lcd_driver.h"

#define Delay osDelay
#define NUM_DIGITS 4

// File specific variables
I2C_HandleTypeDef hi2c1;

//calibStructTypeDef calibStruct;
//calibStoreTypeDef memoryStruct;

int calibration_flag = 0;


//==========================================//
//= Test Function to Replace Calculations  =//
//==========================================//
// - This function should be deleted 				//
//			after integration.									//
// - Test assumes voltage mode with no 			//
//			scaling.														//
// - This can be used as reference for			//
//			integration with the calculation		//
//			code.																//
//==========================================//

double Calib_Conv_Test(int mode, double value, int *rangep, calibAdjustTypeDef *calib_Data) {
	
	// Convert to input voltage
	// This will depend on mode, and extra scaling should be added to 
	// deal with ranges.
	double m = calib_Data[mode].multiplier;
	double c = calib_Data[mode].zeroOffset;
	double output = ((double)value * m) + c;
	
	// Return scaled value
	return output;
}


//==========================================//
//====== Callback for Initialisation  ======//
//==========================================//
// - This function handles clock and GPIO 	//
//			configuration.											//
//																					//
// I2C3 GPIO Configuration:									//
// PA8     ------> I2C3_SCL									//
// PC9     ------> I2C3_SDA									//
//==========================================//

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
		GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_I2C3_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
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


//==========================================//
//======= I2C Initilisation Function =======//
//==========================================//
// - This function handles the 							//
//			configuration of the I2C device.		//
// - The device can be de-initialised with:	//
//			HAL_I2C_DeInit(&hi2c1);							//
//==========================================//

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


//==========================================//
//============ Read Calibration ============//
//==========================================//
// - This function reads calibration points	//
//			from EEPROM and returns a 					//
//			'calibStoreTypeDef' structure 			//
//			containing this data.								//
//==========================================//

void Read_Calibration(calibMeasurementTypeDef *calib_Points) {
	uint8_t address = 0x00;
	uint8_t *calib_Points_Pointer = (uint8_t*)calib_Points;
	int size = sizeof(calibMeasurementTypeDef)*NUM_MODES;
	int i = 0;
	
	// Load stucture
	I2Cinit();
	
	// Set address
	for (i=0; i < size; i+=8) {
		address = i;
		HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, &address, 1, 5000);
		Delay(1);
		// Receive data
		HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<0UL, calib_Points_Pointer, 8, 5000);
		// Convert 'data' array to structure
		calib_Points_Pointer += 8;
		Delay(1);
	}
	
	HAL_I2C_DeInit(&hi2c1);
}


//==========================================//
//=========== Write Calibration  ===========//
//==========================================//
// - This function writes the currently			//
//			stored calibration points to 				//
//			EEPROM.															//
//==========================================//

void Write_Calibration(calibMeasurementTypeDef *calib_Points) {
	uint8_t address = 0x00;
	// TODO: this could probably be simplified
	uint8_t *calib_Points_Pointer = (uint8_t*)calib_Points;
	int size = sizeof(calibMeasurementTypeDef)*NUM_MODES;
	int i, j = 0;
	uint8_t data[9];
	
	I2Cinit();
	
	for (i=0; i < size; i+=8) {
		address = i;
		for (j = 0; j<8; j++) {
			data[j+1] = (calib_Points_Pointer[i+j]);
			data[0] = address;
		}
		HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, data, 9, 5000);
		Delay(10);
	}
	
	HAL_I2C_DeInit(&hi2c1);
}


//==========================================//
//========= Calculate Calibration  =========//
//==========================================//
// - This function calculates calibration		//
//			data based on stored calibration		//
//			points, and stores it locally.			//
//==========================================//

void Calculate_Calibration(calibAdjustTypeDef *calib_Data, calibMeasurementTypeDef *calib_Points) {
	
	//TODO: this needs to be done for each mode.
	
	// y = mx + c
	// output = (input * multiplier) + zero_offset
	
	int i = 0;
	for (i = 0; i < NUM_MODES; i++) {
	
		double x1 = calib_Points[i].lowerPointIn;
		double x2 = calib_Points[i].upperPointIn;
		double y1 = calib_Points[i].lowerPointOut;
		double y2 = calib_Points[i].upperPointOut;
		
		double m = (y2-y1)/(x2-x1);
		double c = y1 - (x1*m);
		
		// TODO: loop for each mode
		calib_Data[i].multiplier = m;
		calib_Data[i].zeroOffset = c;
	}
}


//==========================================//
//============= Factory Reset  =============//
//==========================================//
// - This function resets the stored data		//
//			and EEPROM data to the factory			//
//			defaults.														//
// - These defaults are based on data				//
//			measured in the lab.								//
// - The data is not yet accurate.					//
//==========================================//

void Calibration_Factory_Reset(calibAdjustTypeDef *calib_Data) {

	calibMeasurementTypeDef calib_Points[NUM_MODES];
	
	calib_Points[0].lowerPointIn = 0;
	calib_Points[0].lowerPointOut = -1;
	calib_Points[0].upperPointIn = 3723;
	calib_Points[0].upperPointOut = 1;
	
	calib_Points[1].lowerPointIn = 0;
	calib_Points[1].lowerPointOut = -10;
	calib_Points[1].upperPointIn = 3723; // max raw ADC value
	calib_Points[1].upperPointOut = 10;
	
	calib_Points[2].lowerPointIn = 0;
	calib_Points[2].lowerPointOut = 0;
	calib_Points[2].upperPointIn = 3723;
	calib_Points[2].upperPointOut = 100000;
	
	Write_Calibration(calib_Points);
	Calculate_Calibration(calib_Data, calib_Points);
}


//==========================================//
//======= Calibration Initialisation =======//
//==========================================//
// - This function initialises the 					//
//			calibration code.										//
//==========================================//

void Calibration_Init(calibAdjustTypeDef *calib_Data) {
	calibMeasurementTypeDef calib_Points[NUM_MODES];
	Read_Calibration(calib_Points);
	Calculate_Calibration(calib_Data, calib_Points);
}


//==========================================//
//====== Test Function to Test EEPROM ======//
//==========================================//
// - This function should be deleted 				//
//			after integration.									//
// - This function tests EEPROM writes and	//
//			reads to all required addresses.		//
//==========================================//

void Test_Calibration(calibAdjustTypeDef *calib_Data) {
	char string[17];
	
	Calibration_Init(calib_Data);
	Calibration_Factory_Reset(calib_Data);
	
	calibMeasurementTypeDef calib_Points[NUM_MODES];
	Read_Calibration(calib_Points);
	
	// Print results
	lcd_clear_display();
	sprintf(string, "Current low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[0].lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[0].lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[0].upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Current high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[0].upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[1].lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[1].lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[1].upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Voltage high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[1].upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res low In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[2].lowerPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res low Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[2].lowerPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res high In:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[2].upperPointIn);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	sprintf(string, "Res high Out:");
	lcd_write_string(string, 0, 0);
	sprintf(string, "%1.3lf", calib_Points[2].upperPointOut);
	lcd_write_string(string, 1, 0);
	Delay(2000);
	
	lcd_clear_display();
	
}


//==========================================//
//=========== Calibrate Function ===========//
//==========================================//
// - This function runs the calibration			//
//			routine. It will load the current		//
//			calibration data, display the				//
//			expected output based on current		//
//			data, then allow the user to edit		//
//			this value to reflect an accurate		//
//			reference.													//
//==========================================//

void Calibrate(int mode, int range, calibAdjustTypeDef *calib_Data) {
	char string[17];
	// Load current calibration data
	calibMeasurementTypeDef calib_Points[NUM_MODES];
	Read_Calibration(calib_Points);
	
	// Setup variables
	uint32_t value = 0;
	double target_value = 0;
	uint32_t btns = 0;
	int cursor = 1;
	int pos = 0;
	int exit = 0;
	
	lcd_clear_display();
	
	// Read current value from ADC
	value = read_ADC1();
	//value = (value *16);
	//target_value = adcConv(mode, value, &range);
	target_value = Calib_Conv_Test(mode, value, &range, calib_Data);
	
	// Allow user to adjust expected value
	// TODO: fix this interface
	
	
	// Scale down to proper standard form
	double mantissa = target_value;	
	int exponent = 0;
	int sign = 1;
	if (mantissa != 0) {
		if (mantissa < 0) {
			mantissa = -mantissa;
			sign = 0;
		}
		while (mantissa >= 10) {
			mantissa = mantissa / 10;
			exponent ++;
		}
		while (mantissa < 1) {
			mantissa = mantissa * 10;
			exponent --;
		}
	}
	
	// Convert to set of individual digits for editing
	double temp = mantissa;

	int digits[NUM_DIGITS];

	for (int i = 0; i < NUM_DIGITS; i++) {
		digits[i] = floor(temp);
		temp = (temp - digits[i])*10;
	}
	
	

	
	while (((btns = SWT_Debounce()) != 0x8000) & (exit == 0)) {
		switch (btns) {
			case 0x0100:
				if (cursor > 0) {
					if (cursor == 9)
						cursor = 6;
					cursor --;
					pos --;
					if (cursor == 2)
						cursor --;
				}
			break;
			case 0x0200:
				if (cursor < 6) {
					cursor ++;
					pos ++;
					if (cursor == 2)
						cursor ++;
					if (cursor == 6)
						cursor = 9;
				}
			break;
			case 0x0400:
				// Decrease
				if (cursor == 0) {
					// change sign
					if (sign == 0)
						sign = 1;
					else
						sign = 0;
				}
				else if ((cursor >= 1) & (cursor <= 5)) {
					// change digit
					digits[pos] --;
					if (digits[pos] < 0)
						digits[pos] = 9;
				}
				else {
					// change exponent
					exponent --;
				}
				
			break;
			case 0x0800:
				// Increase
				if (cursor == 0) {
					// change sign
					if (sign == 0)
						sign = 1;
					else
						sign = 0;
				}
				else if ((cursor >= 1) & (cursor <= 5)) {
					// change digit
					digits[pos] ++;
					if (digits[pos] > 9)
						digits[pos] = 0;
				}
				else {
					// change exponent
					exponent ++;
				};
			break;
			case 0x1000:
				for (int i = 0; i < NUM_DIGITS; i++) {
					digits[i] = 0;
				}
				exponent = 0;
				sign = 1;
			break;
			case 0x2000:
				Calibration_Factory_Reset(calib_Data);
				lcd_write_string("Factory Reset", 0, 0);
				Delay(2000);
				exit = 1;
			break;
			case 0x4000:
				lcd_write_string("Back", 0, 0);
				Delay(2000);
				exit = 1;
			break;
			default:
				//blah
			break;
		}
		
		
		if (sign == 0)
			sprintf(string, "-%d.%d%d%d E+%d ", digits[0], digits[1], digits[2], digits[3], exponent);
		else
			sprintf(string, "+%d.%d%d%d E+%d ", digits[0], digits[1], digits[2], digits[3], exponent);
		lcd_write_string(string, 0, 0);
		lcd_write_string("            ", 1, 0);
		lcd_write_string("^", 1, cursor); // TODO: Replace with proper cursor
		Delay(100);
	}
	
	if (exit == 0) {
		
		// Convert digits back to double
		target_value = 0;
		for (int i = NUM_DIGITS-1; i >= 0; i--) {
			target_value = (target_value/10) + digits[i];
		}
		target_value = target_value * pow(10, exponent);
		if (sign == 0)
			target_value = -target_value;
		
		
		
		double half_point = (calib_Points[mode].upperPointIn + calib_Points[mode].lowerPointIn)/2;
		// Write new target value into structure
		if (value < half_point) {
			calib_Points[mode].lowerPointIn = value;
			calib_Points[mode].lowerPointOut = target_value;
			lcd_clear_display();
			lcd_write_string("Lower point set", 0, 0);
			sprintf(string, "%d, %1.3lf", value, target_value);
			lcd_write_string(string, 1, 0);
			Delay(2000);
			lcd_clear_display();
		}
		else {
			calib_Points[mode].upperPointIn = value;
			calib_Points[mode].upperPointOut = target_value;
			lcd_clear_display();
			lcd_write_string("Upper point set", 0, 0);
			sprintf(string, "%d, %1.3lf", value, target_value);
			lcd_write_string(string, 1, 0);
			Delay(2000);
			lcd_clear_display();
		}
		
		// Recalculate convertion
		Calculate_Calibration(calib_Data, calib_Points);
		
		// Write?
		Write_Calibration(calib_Points);
	}
	
	lcd_clear_display();
}


//==========================================//
//================ Set Flag ================//
//==========================================//
// - This function sets a flag so that the	//
//			calibration routine will be run			//
//			next time the flag is checked				//
// - This allows for the calibration				//
//			routine to be triggered from an ISR	//
//			(Interrupt Service Routine).				//
//==========================================//

extern void Set_Calibration_Flag(void) {
	calibration_flag = 1;
}


//==========================================//
//=============== Check Flag ===============//
//==========================================//
// - This function triggers the calibration	//
//			routine if the flag has ben set.		//
//==========================================//

extern void Check_Calibration_Flag(int mode, int range, calibAdjustTypeDef *calib_Data) {
	if (calibration_flag == 1) {
		calibration_flag = 0;
		Calibrate(mode, range, calib_Data);
		calibration_flag = 0;
	}
}


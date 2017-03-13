
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "RTE_Components.h"
#include "ADC.h"
#include "math.h"
#include "System_Init.h"
#include "System_Thread.h"
#include "GPIO.h"
#include "LCD_Thread.h"
#include "Calculations.h"
#include "Serial.h"
#include "String.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay
 

 
 
 
 
void Thread_System (void const *argument);                 // thread function
osThreadId tid_Thread_System;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_System, osPriorityHigh, 1, 0);        // thread object

int Init_Thread_System (void) {

  tid_Thread_System = osThreadCreate(osThread(Thread_System), NULL);
  if (!tid_Thread_System) return(-1);
  
  return(0);
}













// Calibration structures
typedef struct {
	double lowerPoint;
	double upperPoint;
	double zeroOffset;
	double multiplier;
} calibRangeTpeDef;

typedef struct {
	calibRangeTpeDef voltage;
	calibRangeTpeDef current;
	calibRangeTpeDef resistance;
} calibStructTypeDef;


calibStructTypeDef calibStruct;
static const calibStructTypeDef calibStructFlash;
uint8_t *calibStructFlashp = (uint8_t*)&calibStructFlash;



static const double numberFlash = 2.5;
double *numberFlashp = (double*)&numberFlash;







I2C_HandleTypeDef hi2c1;
void I2Cinit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_I2C3_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	/**I2C1 GPIO Configuration
	PA8     ------> I2C3_SCL
	PC9     ------> I2C3_SDA
	*/
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







void Calibrate(int mode, int range) {
	
	uint8_t zero = 0x00;
	int cursor = 0;
	int pos = 0;
	char string[17];
	
	calibStruct = calibStructFlash;
	__HAL_FLASH_DATA_CACHE_DISABLE();
	double number = *numberFlashp;
	__HAL_FLASH_DATA_CACHE_ENABLE();
	
	uint32_t btns = 0;
	LCD_Write_At("", 0, 0, 1);
	
	I2Cinit();
	uint8_t data[9];
	// Set address
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, &zero, 1, 5000);
	// Receive data
	HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<0UL, (uint8_t*)&data, 8, 5000);
	number = *((double*) &data);
	number = 1.0;
	
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
	
	sprintf(string, "Calibrated at:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", number);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	sprintf(string, "Adjusted:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", error);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	
	

	
	/*
	// Send control byte (S1010(xxx0) where xxx = block select
	data = 0xA0;
	HAL_I2C_Master_Transmit(&hi2c1,(0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	// Send address
	data = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1,(0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	// Receive data
	data = 0xFF;
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	
	
	// Send control byte (S1010(xxx0) where xxx = block select
	data = 0xA0;
	HAL_I2C_Master_Transmit(&hi2c1,(0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	// Send address
	data = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1,(0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	// Receive data
	HAL_I2C_Master_Receive(&hi2c1, (0xA0)<<1UL, (uint8_t*)&data, 1, 5000);
	*/
	
	//number = *((double*) &data);
	uint8_t *numpointer = (uint8_t*) &number;
	int i;
	for (i = 0; i<8; i++) {
		data[i+1] = 0xFF & *(numpointer + (8*i));
	}
	data[0] = 0x00;
	
	// Set address and data
	HAL_I2C_Master_Transmit(&hi2c1, (0xA0)<<0UL, data, 9, 5000);
	
	
	// write number at numberFlashp
	sprintf(string, "I2C:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.4lf", number);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	
	// write number at numberFlashp
	sprintf(string, "Written to:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "0x%x", (uint32_t)numberFlashp);
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	
		// write number at numberFlashp
		/*
	sprintf(string, "Length:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%d", sizeof(number));
	LCD_Write_At(string, 0, 1, 0);
	Delay(2000);
	LCD_Write_At("", 0, 0, 1);
	*/
	
	//*numberFlashp = number;
	/*
	HAL_StatusTypeDef ret;
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	HAL_FLASH_Unlock();
	ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)numberFlashp, (uint64_t)number);
	HAL_FLASH_Lock();
	if (ret != 0) {
		// print error
		sprintf(string, "Write error:");
		LCD_Write_At(string, 0, 0, 1);
		sprintf(string, "%d", ret);
		LCD_Write_At(string, 0, 1, 0);
		Delay(2000);
		LCD_Write_At("", 0, 0, 1);
		
		uint32_t error = HAL_FLASH_GetError();
		
		if(((error >> 0) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "RDERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		if(((error >> 1) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "PGSERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		if(((error >> 2) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "PGPERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		if(((error >> 3) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "PGAERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		if(((error >> 4) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "WRPERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		if(((error >> 5) & 1UL) == 1UL) {
			sprintf(string, "Write error:");
			LCD_Write_At(string, 0, 0, 1);
			sprintf(string, "OPERR");
			LCD_Write_At(string, 0, 1, 0);
			Delay(2000);
			LCD_Write_At("", 0, 0, 1);
		}
		
	}
	*/
	
}














void Thread_System (void const *argument) {
	
	Delay(100); // wait for mpool to be set up in other thread (some signaling would be better)
	
	//SerialInit();
	//SerialReceiveStart();
		
	uint32_t value = 0;
	double value_calk = 0;
	char string[17];
	char unit[2] = {'A', '\0'};
	
	// Ranging perameters
	int range = 0; // lower = larger range / lower resolution (for Amps)
	int mode = 0; // C, V, R

	
	GPIOD->ODR = 0;
	LCD_Write_At(NULL, 0, 0, 1);

	while (1) {
		uint32_t btns = 0;
		
		// Read mode
		btns = SWT_Debounce();
		
		switch (btns) {
			case 0x0100:
				mode = 0;
			break;
			case 0x0200:
				mode = 1;
			break;
			case 0x0400:
				mode = 2;
			break;
			case 0x8000:
				Calibrate(mode, range);
			break;
			default:
				//blah
			break;
		}
		
		switch (mode) {
			case 0:
				unit[0] = 'A';
			break;
			case 1:
				unit[0] = 'V';
			break;
			case 2:
				unit[0] = (char)0xDE;
			break;
			default:
				sprintf(string, "Undefined mode!");
				LCD_Write_At(string, 0, 0, 0);
				Delay(1000);
			break;
		}
		
		// Read ADC
		value = read_ADC1();
		value = (value *16);
		
		value_calk = adcConv(mode, value, &range);

		/*
		// Switch range based on limits
		if ((value_calk > InnerLowerLimit) & (value_calk < InnerUpperLimit)){
			if (range < maxRange) {
				range++;
			}
			else {
				// TODO: Print error to LCD
			}
		}
		else if ((value_calk > OuterUpperLimit) | (value_calk < OuterLowerLimit)) {
			if (range > minRange) {
				range--;
			}
			else {
				// TODO: Print error to LCD
			}
		}
		*/
		
		
		
		// Set output based on range
		switch (range) {
			case 0:
				GPIO_Off(0);
			break;
			case 1:
				GPIO_On(0);
			break;
			default:
				GPIO_Off(0); // Disconnect all inputs if possible
			break;
		}
		
		// Put to LCD
		/*
		//LCD_Clear();
		LCD_GotoXY(0,0);
		sprintf(string, "                ");
		sprintf(string, "%1.9lf", value_calk);
		LCD_PutS(string);
		LCD_GotoXY(15,0);
		LCD_PutS(unit);
		Delay(100);
		*/
		

		sprintf(string, "%1.9lf", value_calk);
		LCD_Write_At(string, 0, 0, 0);
		LCD_Write_At(unit, 15, 0, 0);
		
		if (range == 1) {
			LCD_Write_At("m", 14, 0, 0);
			sprintf(string, "%s m%s\r\n", string, unit);
		} else {
			LCD_Write_At(" ", 14, 0, 0);
			sprintf(string, "%s %s\r\n", string, unit);
		}

		//SerialSend((uint8_t*)string, strlen(string), 1000);
		
		//SerialReceive();
		
		//SerialCheckMode(&mode);

		
	}
}

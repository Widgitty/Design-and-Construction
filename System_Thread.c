
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
#include "stm32f4xx_hal_tim.h"
#include "Timers.h"
#include "cmsis_os.h"
#include "core_cm4.h"
#include "Calculations.h"
#include "Serial.h"
#include "String.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay
#define resistance 10000;
 
static TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};

void Thread_System (void const *argument);                 // thread function
osThreadId tid_Thread_System;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_System, osPriorityHigh, 1, 0);        // thread object

int Init_Thread_System (void) {

  tid_Thread_System = osThreadCreate(osThread(Thread_System), NULL);
  if (!tid_Thread_System) return(-1);
  
  return(0);
}




void Calibrate(int mode, int range) {
	int cursor = 0;
	int pos = 0;
	double number = 1.234;
	char string[17];
	
	uint32_t btns = 0;
	LCD_Write_At("", 0, 0, 1);
	
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
}



void Thread_System (void const *argument) {
	Delay(100); // wait for mpool to be set up in other thread (some signaling would be better)
	
	char string[17];
	SerialInit();
	SerialReceiveStart();
	
	
	
	
	
	//HAL_TIM_Base_Start(&timer_Instance);
	
	// unreachable code below
	
	
	
	uint32_t value = 0;
	double value_calk = 0;
	char unit[2] = {'A', '\0'};
	
	// Ranging perameters
	int range = 0; // lower = larger range / lower resolution (for Amps)
	int mode = 0; // C, V, R

	
	
	GPIOD->ODR = 0;
	LCD_Write_At(NULL, 0, 0, 1);


	while (1) {
		uint32_t btns = 0;
		Delay(10);
		
		// Read mode
		btns = SWT_Debounce();
		
		switch (btns) {
			case 0x0100:

				LED_Out(1);
				mode = 0;
			break;
			case 0x0200:
				LED_Out(2);
				mode = 1;
			break;
			case 0x0400:
				LED_Out(4);
				mode = 2;
			break;
			case 0x0800:
				LED_Out(8);
				mode = 3;
				capacitorState = 0;

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
				//nada
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
			case 3:
				unit[0] = 'F';
			break;
			default:
				unit[0] = '/';
				sprintf(string, "Undefined mode!");
				LCD_Write_At(string, 0, 0, 0);
				Delay(1000);
			break;
		}
		
		// Read ADC
		value = read_ADC1();
		value = (value *16);
		
		value_calk = adcConv(mode, value, &range);

		value_calk = movAvg(value_calk, mode, &range);

		
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

		sprintf(string, "%1.9lf", value_calk);
		LCD_Write_At(string, 0, 0, 0);
		LCD_Write_At(unit, 15, 0, 0);
		
		if (range == 1 && (mode == 0 || mode == 1)) {
			LCD_Write_At("m", 14, 0, 0);
			sprintf(string, "%s m%s\r\n", string, unit);
		}
		
		if (range == 1 && mode == 2) {
			LCD_Write_At("K", 14, 0, 0);
			sprintf(string, "%s m%s\r\n", string, unit);
		}
		
	  if (range == 0){
			LCD_Write_At(" ", 14, 0, 0);
			sprintf(string, "%s %s\r\n", string, unit);
		}

		SerialSend((uint8_t*)string, strlen(string), 1000);
		
		SerialReceive();
		
		SerialCheckMode(&mode);

		
	}
}


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


void Thread_System (void const *argument) {
	
//	osPoolDef(mpool, 16, char[17]);
//	osPoolId mpool;
	
	char string[17] = {'A', 'B', 'C', '\0'};
	char *stringp = string;
	
	while (1) {
		osMessagePut(MsgBox, (uint32_t)stringp, osWaitForever);
		Delay(1000);
	}
	
	/*
	uint32_t value = 0;
	double value_calk = 0;
	char string[17];
	char unit[2] = {'A', '\0'};
	
	// Ranging perameters
	int range = 0; // lower = larger range / lower resolution (for Amps)
	int mode = 0; // C, V, R
	float OuterUpperLimit = 2.5;
	float OuterLowerLimit = 0.5;
	float InnerUpperLimit = 1.6;
	float InnerLowerLimit = 1.4;
	int maxRange = 1;
	int minRange = 0;
	
	GPIOD->ODR = 0;

	while (1) {
		uint32_t btns = 0;
		
		// Read mode
		btns = SWT_Debounce();
		
		switch (btns) {
			case 0x0100:
				unit[0] = 'A';
				mode = 0;
			break;
			case 0x0200:
				unit[0] = 'V';
				mode = 1;
			break;
			case 0x0400:
				unit[0] = (char)0xDE;
				mode = 2;
			break;
			default:
				//blah
			break;
		}
		
		// Read ADC
		value = read_ADC1();
		value = (value *16);
		GPIOD->ODR = value;
		
		value_calk = ((double)value / (pow(2.0, 16.0))) * 3.3;
		//value_calk = (double)0.5;
		
		// Convert to value
		// insert fomula here
		
		// Switch range based on limits
		if ((value_calk > InnerLowerLimit) & (value_calk < InnerUpperLimit)){
			if (range < maxRange) {
				range++;
			}
			else {
				// Print error to LCD
			}
		}
		else if ((value_calk > OuterUpperLimit) | (value_calk < OuterLowerLimit)) {
			if (range > minRange) {
				range--;
			}
			else {
				// Print error to LCD
			}
		}
		
		
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
		//LCD_Clear();
		LCD_GotoXY(0,0);
		sprintf(string, "                ");
		sprintf(string, "%1.9lf", value_calk);
		LCD_PutS(string);
		LCD_GotoXY(15,0);
		LCD_PutS(unit);
		LCD_GotoXY(0,1);
		sprintf(string, "%d", mode);
		LCD_PutS(string);
		Delay(100);
		
	}
	*/
}

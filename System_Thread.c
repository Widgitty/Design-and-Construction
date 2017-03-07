
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


void Thread_System (void const *argument) {
	
	Delay(100); // wait for mpool to be set up in other thread (some signaling would be better)
	
	SerialInit();
	SerialReceiveStart();
		
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
		value_calk = movAvg(value_calk, mode);
	
		
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
		
		if (range == 1 && mode != 2) {
			LCD_Write_At("m", 14, 0, 0);
			sprintf(string, "%s m%s\r\n", string, unit);
		}
		
		if (range == 1 && mode == 2) {
			LCD_Write_At("K", 14, 0, 0);
			sprintf(string, "%s m%s\r\n", string, unit);
		}
	  else {
			LCD_Write_At(" ", 14, 0, 0);
			sprintf(string, "%s %s\r\n", string, unit);
		}

		SerialSend((uint8_t*)string, strlen(string), 1000);
		
		SerialReceive();
		
		SerialCheckMode(&mode);

		
	}
}

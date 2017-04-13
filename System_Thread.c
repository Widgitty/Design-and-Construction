
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "LED.h"
#include "SWT.h"
#include "RTE_Components.h"
#include "ADC.h"
#include "math.h"
#include "System_Init.h"
#include "System_Thread.h"
#include "GPIO.h"
#include "stm32f4xx_hal_tim.h"
#include "Timers.h"
#include "cmsis_os.h"
#include "core_cm4.h"
#include "Calculations.h"
#include "Serial.h"
#include "String.h"
#include "lcd_driver.h"
#include "Defines.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay
#define resistance 10000;
 
static TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};

void Thread_System (void const *argument);                 // thread function
osThreadId tid_Thread_System;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_System, osPriorityHigh, 1, 0);        // thread object
uint32_t buttonUpdate = 0;

int Init_Thread_System (void) {
  tid_Thread_System = osThreadCreate(osThread(Thread_System), NULL);
  if (!tid_Thread_System) return(-1);
  return(0);
}

void Set_Button_Update(void){
	buttonUpdate = 1;
}

void Thread_System (void const *argument) {
	Delay(100); // wait for mpool to be set up in other thread (some signaling would be better)
	
	char string[17];
	SerialInit();
	SerialReceiveStart();
	
	uint32_t value = 0;
	double value_calk = 0;
	char unit[2] = {'A', '\0'};
	
	// Ranging perameters
	int range = 0; // lower = larger range / lower resolution (for Amps)
	int mode = 0; // C, V, R, F, H

	
	
	GPIOD->ODR = 0;


	while (1) {
		Delay(10);
		
		
		
		// this code is only executed if a button update happened (a button was pressed)
		
		if(buttonUpdate == 1){
			buttonUpdate = 0;
			mode = Get_Mode();
			switch (mode) {
				case 0:
					unit[0] = 'A';
					LED_Out(1);
					lcd_write_string("              ", 0,0);
				break;
				case 1:
					unit[0] = 'V';
					LED_Out(2);
					lcd_write_string("              ", 0,0);
				break;
				case 2:
					unit[0] = (char)0xDE;
					LED_Out(4);
					lcd_write_string("              ", 0,0);
				break;
				case 3:
					unit[0] = 'F';
					LED_Out(8);
					capacitorState = 0;
					lcd_write_string("              ", 0,0);
				break;
				case 4: 
					unit[0] = 'H';
					LED_Out(16);
					lcd_write_string("              ", 0,0);
				break;
				case 5:
					unit[0] = 'H';
					LED_Out(32);
					lcd_write_string("              ", 0,0);
				break;
				default:
					unit[0] = '/';
					sprintf(string, "Undefined mode!");
					lcd_write_string(string, 0,0);
					Delay(1000);
				break;
			}
		}
		
		// Read ADC
		value = read_ADC1();
		value = (value *16);
		
		value_calk = adcConv(mode, value, &range);
		
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
		lcd_write_string(string, 0, 0);
		lcd_write_string(unit, 0, 15);
		
		switch(range)
		{
			case nano:
				lcd_write_string("n", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case micro:
				lcd_write_string("u", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case milli:
				lcd_write_string("m", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case nothing:
				lcd_write_string(" ", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case kilo:
				lcd_write_string("k", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case mega:
				lcd_write_string("M", 0, 14);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
		}
		/*
		if (range == 1) {
			lcd_write_string("m", 0, 14);
			sprintf(string, "%s m%s\r\n", string, unit);
		} else {
			lcd_write_string(" ", 0, 14);
			sprintf(string, "%s %s\r\n", string, unit);
		}*/

		SerialSend((uint8_t*)string, strlen(string), 1000);
		
		SerialReceive();
		
		SerialCheckMode(&mode);

		
	}
}

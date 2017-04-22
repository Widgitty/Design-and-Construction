
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
void resetTimersAndStates(void);
void getButtonUpdate(void);

void Thread_System (void const *argument);                 // thread function
osThreadId tid_Thread_System;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_System, osPriorityHigh, 1, 0);        // thread object
uint32_t buttonUpdate = 0;
int mode = 0; // C, V, R, F, H
char unit[3] = {'A',' ', '\0'};
char string[17];
int Init_Thread_System (void) {
  tid_Thread_System = osThreadCreate(osThread(Thread_System), NULL);
  if (!tid_Thread_System) return(-1);
  return(0);
}

void Set_Button_Update(void){
	buttonUpdate = 1;
}
void getButtonUpdate(void){
	resetTimersAndStates();
	buttonUpdate = 0;
	mode = Get_Mode();
	switch (mode) {
		case 0:
			unit[0] = 'A';
			unit[1] = ' ';
			LED_Out(1);
			lcd_write_string("              ", 0,0);
		break;
		case 1:
			unit[0] = 'V';
			unit[1] = ' ';
			LED_Out(2);
			lcd_write_string("              ", 0,0);
		break;
		case 2:
			unit[0] = (char)0xDE;
			unit[1] = ' ';
			LED_Out(4);
			lcd_write_string("              ", 0,0);
		break;
		case 3:
			unit[0] = 'F';
			unit[1] = ' ';
			LED_Out(8);
			capacitorState = 0;
			lcd_write_string("              ", 0,0);
		break;
		case 4: 
			unit[0] = 'H';
			unit[1] = ' ';
			LED_Out(16);
			inductanceState = 0;
			lcd_write_string("              ", 0,0);
		break;
		case 5:
			unit[0] = 'H';
			unit[1] = 'z';
			LED_Out(32);
			frequencyState = 0;
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

void Thread_System (void const *argument) {
	Delay(100); // wait for mpool to be set up in other thread (some signaling would be better)
	
	
	SerialInit();
	SerialReceiveStart();
	
	uint32_t value = 0;
	double value_calk = 0;
	
	
	// Ranging perameters
	// range defines the relay output, which is on when being on the milli range and also sets the LCD to show a certain value.
	int range = nothing; 
	

	
	
	GPIOD->ODR = 0;


	while (1) {
		Delay(10);
		
		
		
		// this code is only executed if a button update happened (a button was pressed)
		
		if(buttonUpdate == 1){
			getButtonUpdate();
		}
		
		// Read ADC
		value = read_ADC1();
		value = (value *16);
		
		value_calk = adcConv(mode, value, &range);
		
		// Set output based on range
		switch (range) {
			case milli:
				GPIO_On(0);
			break;
			case nothing:
				GPIO_Off(0);
			break;
			default:
				GPIO_Off(0); // Disconnect all inputs if possible
			break;
		}

		sprintf(string, "%1.3lf   ", value_calk);
		lcd_write_string(string, 0, 0);
		lcd_write_string(unit, 0, 14);
		
		switch(range)
		{
			case nano:
				lcd_write_string("n", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case micro:
				lcd_write_string("u", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case milli:
				lcd_write_string("m", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case nothing:
				lcd_write_string(" ", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case kilo:
				lcd_write_string("k", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
			case mega:
				lcd_write_string("M", 0, 13);
				sprintf(string, "%s m%s\r\n", string, unit);
			break;
		}

		SerialSend((uint8_t*)string, strlen(string), 1000);
		
		SerialReceive();
		
		SerialCheckMode(&mode);

		
	}
}
void resetTimersAndStates(void){
	capacitorState = 0;
	inductanceState = 0;
	frequencyState = 0;
	HAL_TIM_Base_Stop(&timer_Instance_1);
	__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
	HAL_TIM_Base_Stop(&timer_Instance_2);
	__HAL_TIM_SET_COUNTER(&timer_Instance_2, 0);
	HAL_TIM_Base_Stop(&timer_Instance_3);
	__HAL_TIM_SET_COUNTER(&timer_Instance_3, 0);
	HAL_TIM_Base_Stop(&timer_Instance_4);
	__HAL_TIM_SET_COUNTER(&timer_Instance_4, 0);
}

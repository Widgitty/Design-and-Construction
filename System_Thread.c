
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "LED.h"
#include "SWT.h"
#include "RTE_Components.h"
#include "ADC.h"
#include "DAC.h"
#include "math.h"
#include "System_Init.h"
#include "System_Thread.h"
#include "GPIO.h"
#include "stm32f4xx_hal_tim.h"
#include "Timers.h"
#include "cmsis_os.h"
#include "core_cm4.h"
#include "Calculations.h"
#include "String.h"
#include "Calibration.h"
#include "lcd_driver.h"
#include "Defines.h"
#include "Comms.h"


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

uint32_t modeUpdate = 1;


int mode = 0; // C, V, R, F, H, Hz
int muxRange = 0;
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
	GPIO_SetMode(mode);

	switch (mode) {
		case CURRMODE:
			unit[0] = 'A';
			unit[1] = ' ';
			LED_Out(1);
			lcd_write_string("Current", 0,0);
		break;
		case VOLTMODE:
			unit[0] = 'V';
			unit[1] = ' ';
			LED_Out(2);
			lcd_write_string("Voltage", 0,0);
		break;
		case RESMODE:
			unit[0] = (char)0xDE;
			unit[1] = ' ';
			LED_Out(4);
			lcd_write_string("Resistance", 0,0);
		break;
		case CAPMODE:
			unit[0] = 'F';
			unit[1] = ' ';
			LED_Out(8);
			capacitorState = 0;
			lcd_write_string("Capacitance", 0,0);
		break;
		case INDMODE: 
			unit[0] = 'H';
			unit[1] = ' ';
			LED_Out(16);
			inductanceState = 0;
			lcd_write_string("Inductance", 0,0);
		break;
		case FREQMODE:
			unit[0] = 'H';
			unit[1] = 'z';
			LED_Out(32);
			frequencyState = 0;
			lcd_write_string("Frequency", 0,0);
		break;
		case DIODE:
			LED_Out(4);
			lcd_write_string("Diode mode", 0,0);
			break;
		case CONTMODE:
			LED_Out(4);
			lcd_write_string("Continuity mode", 0,0);
			break;
		case RMS:
			LED_Out(2);
			lcd_write_string("Voltage RMS", 0,0);
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

	char string[17];

	Comms_Init();

	calibAdjustTypeDef calib_Data[NUM_MODES];
	Calibration_Init(calib_Data);

	
	uint32_t value = 0;
	double value_calk = 0;
	
	
	// Ranging perameters
 
	LED_Out(1);
	GPIO_Off(3);
 
	// range defines the relay output, which is on when being on the MILLI range and also sets the LCD to show a certain value.
	int range = UNIT; 

	
	GPIOD->ODR = 0;
	getButtonUpdate();

	while (1) {

		Delay(10);


		// this code is only executed if a button update happened (a button was pressed)
		if(buttonUpdate == 1){
			buttonUpdate = 0;
			mode = Get_Mode();
			modeUpdate = 1;
		}
		
		if (modeUpdate == 1) {
			modeUpdate = 0;

			getButtonUpdate();

		}
		
		// Read ADC
		Delay(10);
		//if(mode != CONTMODE && mode != DIODE){
		if (1) {
			value = read_ADC1();
			
			//value_calk = value;
			
			value_calk = adcConv(mode, value, &range, calib_Data);
			
			//value_calk = movAvg(value_calk, mode, &range);
			
			//value_calk = Calib_Conv_Test(mode, value, &range, calib_Data);

			
			// Set output based on range
			// !!!!!! this may do the same job as the mode muyxing atm
			
			/*
			switch (range) {
				case MILLI:
					GPIO_On(0);
				break;
				case UNIT:
					GPIO_Off(0);
				break;
				default:
					GPIO_Off(0); // Disconnect all inputs if possible
				break;
			}
			*/
		



		
			
			sprintf(string, "%1.3lf   ", value_calk);
			lcd_write_string(string, 1, 0);
			lcd_write_string(unit, 1, 14);
		
		
			switch(range)
			{
				case NANO:
					lcd_write_string("n", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
				break;
				case MICRO:
					lcd_write_string("u", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
				break;
				case MILLI:
					lcd_write_string("m", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
				break;
	 
				case UNIT:
					lcd_write_string(" ", 1, 13);
	 
	 
					sprintf(string, "%s m%s\r\n", string, unit);
					GPIO_Off (3);
				break;
				case UNIT30:
					lcd_write_string(" ", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
					GPIO_On (3);
				break;
				case KILO:
					lcd_write_string("k", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
				break;
				case MEGA:
					lcd_write_string("M", 1, 13);
					sprintf(string, "%s m%s\r\n", string, unit);
				break;
			}
		}

		/*
		if (range == 1) {
			lcd_write_string("m", 0, 14);
			sprintf(string, "%s m%s\r\n", string, unit);
		} else {
			lcd_write_string(" ", 0, 14);
			sprintf(string, "%s %s\r\n", string, unit);
		}*/

		//SerialSend((uint8_t*)string, strlen(string));
		Comms_Send(value_calk, (uint8_t)mode, (uint8_t)range);
		
		Comms_Receive();
		
		int modePrev = mode;
		Comms_Check_Mode(&mode);
		if (modePrev != mode) {
			modeUpdate = 1;
		}

		
		Check_Calibration_Flag(mode, range, calib_Data);
	}
}


void resetTimersAndStates(void){
	lcd_write_string("                ", 0,0);
	lcd_write_string("                ", 1,0);
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

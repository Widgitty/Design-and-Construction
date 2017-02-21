
#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "cmsis_os.h"
#include "RTE_Components.h"
#include "System_Init.h"

#include "math.h"
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "ADC.h"
#include "GPIO.h"

#include "Serial.h"
#include "String.h"


// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	
	osKernelInitialize();     
	
  HAL_Init();
	
  SystemClock_Config();
	
  LED_Init();  
  SWT_Init();
	ADC1_Init();
	GPIO_Init();
	
	
  LCD_Initpins();	
	LCD_DriverOn();
	Delay(10);
	LCD_Init();
	LCD_DriverOn();
	LCD_On(1);
	
	//Init_Thread_System();
	//Init_Thread_LCD();
	
	char string[17];
	sprintf(string, "HELLO\r\n");

	LCD_Clear();
	LCD_GotoXY(0, 0);
	LCD_PutS(string);
	
	SerialInit();
	SerialSend((uint8_t*)string, 17, 1000);

	SerialReceiveStart();
	while (1) {
		SerialReceive();
		Delay(5000);
	}

}

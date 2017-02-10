
#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "cmsis_os.h" 
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "RTE_Components.h"
#include "ADC.h"
#include "math.h"
#include "System_Init.h"
#include "System_Thread.h"

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
	
	
  LCD_Initpins();	
	LCD_DriverOn();
	Delay(10);
	LCD_Init();
	LCD_DriverOn();
	LCD_On(1);
	
	Init_Thread_System();

}


#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "cmsis_os.h"
#include "RTE_Components.h"
#include "System_Init.h"
#include "System_Thread.h"
#include "math.h"
#include "LED.h"
#include "SWT.h"
#include "ADC.h"
#include "GPIO.h"
#include "Timers.h"
#include "lcd_driver.h"



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
	Timer_Init();
	Interrupt_Init();
	
	lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, 128);
	lcd_clear_display();
	
	Delay(10);
	Init_Thread_System();

}

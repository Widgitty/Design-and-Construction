
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


void testing (void)
{
	uint32_t btns = 0;
	uint32_t leds = 0;
	while(1) {                                    /* Loop forever               */
		btns = SWT_Debounce();                           /* Read switch states         */
		if (btns != 0)
		{
			leds = leds ^ btns; 
			GPIOD->ODR = leds;
			Delay(100);
			
			LCD_Clear();
			LCD_GotoXY(0,0);
			
			switch(leds >> 8){
				case 0:
					LCD_PutS("No LEDs");
					break;
				case 1:
					LCD_PutS("LED 1");
					break;
				case 2:
					LCD_PutS("LED 2");
					break;
				case 4:
					LCD_PutS("LED 3");
					break;
				case 8:
					LCD_PutS("LED 4");
					break;
				case 16:
					LCD_PutS("LED 5");
					break;
				case 32:
					LCD_PutS("LED 6");
					break;
				case 64:
					LCD_PutS("LED 7");
					break;
				case 128:
					LCD_PutS("LED 8");
					break;

				default:
					LCD_PutS("Multiple");
					LCD_GotoXY(0,1);
					LCD_PutS("LEDs");
					break;
				
			}
		}
	}
}

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

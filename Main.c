
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

	
	uint32_t value = 0;
	double value_calk = 0;
	char string[17];
	
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }

	GPIOD->ODR = 0;																/* turn LEDs off */

	//testing();
	
	while (1) {
		value = read_ADC1();
		value = (value *16);
		GPIOD->ODR = value;
		
		value_calk = ((double)value / (pow(2.0, 16.0))) * 3.3;
		//value_calk = (double)0.5;
		
		LCD_Clear();
		LCD_GotoXY(0,0);
		sprintf(string, "                ");
		sprintf(string, "%1.9lf", value_calk);
		LCD_PutS(string);
		LCD_GotoXY(15,0);
		LCD_PutS("V");
		Delay(100);
	}	
}

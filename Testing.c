#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "cmsis_os.h" 
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "ADC.h"

#define Delay osDelay

void UITest (void)
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


void LCDCharFind(void)
{
	char string[17];
	char ohms[2] = {(char)0xDE, '\0'};
	uint8_t count = 0x00;
	while (1) {
		LCD_Clear();
		LCD_GotoXY(0,0);
		sprintf(string, "                ");
		sprintf(string, "0x%02x", count);
		LCD_PutS(string);
		LCD_GotoXY(15,0);
		ohms[0] = 0xDE;
		LCD_PutS(ohms);
		Delay(100);
		count++;
	}
}

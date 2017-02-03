/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: MOdification to Drive 2x16 LCD
 * Note(s): 
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
 
 /* MODIFIED BY D. CHESMORE JANUARY 2013   */
 
#include <stdio.h>
#include "STM32F4xx.h"
#include "LED.h"
#include "SWT.h"
#include "LCD.h"



volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */

/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;
}

/*----------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *----------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {                                              
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}
 




/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }

  LED_Init();  
  SWT_Init();	
  LCD_Initpins();	
	LCD_DriverOn();
	
	Delay(10);
	LCD_Init();

	LCD_DriverOn();
	LCD_On(1);

	uint32_t btns = 0;
	uint32_t leds = 0;
	
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }

	GPIOD->ODR = 0;																/* turn LEDs off */
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


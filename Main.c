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
#include "STM32F4xx_hal.h"
#include "cmsis_os.h" 
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "RTE_Components.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay


#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
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
	/*while (1) {
		GPIOD->ODR = 1UL << 8;
		Delay(1000);
		GPIOD->ODR = 0;
		Delay(1000);
	}*/

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


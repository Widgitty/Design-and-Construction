/*----------------------------------------------------------------------------
 * Name:    SWT.c
 * Purpose: low level SWT 'input switch' functions
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

#include "STM32F4xx.h"
#include "SWT.h"
#include "LED.h"
#include "LCD.h"
#include "System_Thread.h"
#include "Calculations.h"
#include "Timers.h"

const unsigned long SWT_mask[] = {1UL << 8, 1UL << 9, 1UL << 10, 1UL << 11, 1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15};
void init_swt_interrupt(void);
void Calc_Temp_Mode(void);
uint32_t Get_Mode(void);
	uint32_t prev = 0;
	uint32_t temp_mode = 0;
	uint32_t GPIOE_value;
	uint32_t a = 0;

/*----------------------------------------------------------------------------
  initialize SWT Pins
 *----------------------------------------------------------------------------*/
void SWT_Init (void) {
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->AHB1ENR    |=  RCC_AHB1ENR_GPIOEEN;
	
  GPIOE->MODER    &= ~((3UL << 2* 8) |
                       (3UL << 2* 9) |
                       (3UL << 2*10) |
                       (3UL << 2*11) |
                       (3UL << 2*12) |
                       (3UL << 2*13) |
                       (3UL << 2*14) |
                       (3UL << 2*15)  );   /* PE.8..15 is input               */
  GPIOE->OSPEEDR  &= ~((3UL << 2* 8) |
                       (3UL << 2* 9) |
                       (3UL << 2*10) |
                       (3UL << 2*11) |
                       (3UL << 2*12) |
                       (3UL << 2*13) |
                       (3UL << 2*14) |
                       (3UL << 2*15)  );   /* PE.8..15 is 50MHz Fast          */
  GPIOE->PUPDR    &= ~((3UL << 2* 8) |
                       (3UL << 2* 9) |
                       (3UL << 2*10) |
                       (3UL << 2*11) |
                       (3UL << 2*12) |
                       (3UL << 2*13) |
                       (3UL << 2*14) |
                       (3UL << 2*15)  );   /* PE.8..15 is no Pull up             */
	GPIOE->PUPDR    |= 	((2UL << 2* 8) |
                       (2UL << 2* 9) |
                       (2UL << 2*10) |
                       (2UL << 2*11) |
                       (2UL << 2*12) |
                       (2UL << 2*13) |
                       (2UL << 2*14) |
                       (2UL << 2*15)  );   /* PE.8..15 set pull down             */
	
	
	//port e listener for ports 8-15
	SYSCFG->EXTICR[2] |= 0x4444;
	SYSCFG->EXTICR[3] |= 0x4444;
	
	//sets bits 8-15 high for interrupts on each button
	EXTI->RTSR |= 0xFF << 8;
	EXTI->IMR |= 0xFF << 8;
	
	// enable IRG functions for interrupts
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// interrupt functions 
void EXTI15_10_IRQHandler(void){
	
	
	
	if(GPIOE->IDR != 0x0000000C){
		EXTI->PR |= 1;
		//just need top 8 bits
		GPIOE_value = GPIOE->IDR >> 8;
		Calc_Temp_Mode();
		// sets a value that the buttons just got updated
		Set_Button_Update();
	}
	
	
	
}

void EXTI9_5_IRQHandler(void){
	
	
	a = 0x00000020 & GPIOB->IDR;
	
	if(a == 0x00000020)
	{
		EXTI->PR |= 1 << 5;
		// counts up to calculate frequency
		if(frequencyState == 1){
			countUp();
		}
		else if(inductanceState == 1){
			setTimerValue(__HAL_TIM_GET_COUNTER(&timer_Instance_3));
		}
	}
	
	
	else if(GPIOE->IDR != 0x0000000C){
		EXTI->PR |= 0xFFFF << 8;
		//just need top 8 bits
		GPIOE_value = GPIOE->IDR >> 8;
		Calc_Temp_Mode();
		// sets a value that the buttons just got updated
		Set_Button_Update();
	}
}
//calculates a temporary mode that then gets called each time from the System_Thread.
void Calc_Temp_Mode(void){
	
	switch(GPIOE_value){
		case 1:
			temp_mode = 0;
			break;
		case 2:
			temp_mode = 1;
			break;
		case 4:
			temp_mode = 2;
			break;
		case 8:
			temp_mode = 3;
			break;
		case 16:
			temp_mode = 4;
			break;
		case 32:
			temp_mode = 5;
			break;
		default:
			// nothing
			break;
	}
}

// function returns the temporary mode 
uint32_t Get_Mode(void){
	return temp_mode;
}

/*----------------------------------------------------------------------------
  Function that reads Switch states
 *----------------------------------------------------------------------------*/
uint32_t SWT_Get (void) {

  return (GPIOE->IDR);
}
/*----------------------------------------------------------------------------
  Debouncer
 *----------------------------------------------------------------------------*/
uint32_t SWT_Debounce() {
	uint32_t btns = 0;
	uint32_t out = 0;
	
	btns = SWT_Get();
	out = (btns ^ prev) & btns;
	prev = btns;
	return out;
}

void init_swt_interrupt(void){
}

/*----------------------------------------------------------------------------
  Function that checks the state of requested switch
 *----------------------------------------------------------------------------*/
int SWT_Check (unsigned int num) {
	
	if ( num > SWT_NUM )
		return 0;
	else
		return ( GPIOE->IDR & SWT_mask[num] );
}

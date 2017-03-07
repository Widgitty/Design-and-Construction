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

const unsigned long SWT_mask[] = {1UL << 8, 1UL << 9, 1UL << 10, 1UL << 11, 1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15};
void init_swt_interrupt(void);
	uint32_t prev = 0;

/*----------------------------------------------------------------------------
  initialize SWT Pins
 *----------------------------------------------------------------------------*/
void SWT_Init (void) {

  RCC->AHB1ENR    |=  ((1UL <<  4)    );   /* Enable GPIOE clock              */

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

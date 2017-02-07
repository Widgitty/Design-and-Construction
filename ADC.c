/*----------------------------------------------------------------------------
 * Name:    ADC_LEDs.c

reads ADC channel and displays upper 8 bits (of 12) on LEDs*/


#include <stdio.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay



/* Function to intiialise ADC1    */

void ADC1_Init(void) {
	
	RCC->APB2ENR  |= ((1UL <<  8) );         /* Enable ADC1 clock                */
	RCC->AHB1ENR  |= ((1UL <<  2) );         /* Enable GPIOC clock                */
	GPIOC->MODER = 0xffffffff;
	GPIOC->PUPDR = 0;
	ADC1->CR1 = 0x00;
	ADC1->CR1 |= (1UL << 11);
	ADC1->CR2 = 0x00;
	ADC1->CR2 |= (1UL << 10) ;					/* right alignement of 12 bits */
	ADC->CCR = 0x00;
	ADC1->SQR1 = 0x01;								/* 1 conversion at a time */
	ADC1->SMPR1 = 0x00;
	ADC1->SMPR1 = 0x0300;
	ADC1->SQR1 = 0x01;
	ADC1->SQR3 = 0x0e;								/* ADC_IN14 = 0x0e: ADC_IN15 = 0x0f */
	ADC1->CR2 |= (1UL << 0);
	
}
	
/* function to read ADC and retun value */
unsigned int read_ADC1 (void) {
	
	ADC1->CR2 |= (1UL << 30)	;		/* set SWSTART to 1 to start conversion */
	Delay(100);
	return (ADC1->DR);
}


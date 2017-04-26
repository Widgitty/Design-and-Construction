/*----------------------------------------------------------------------------
 * Name:    DAC_LEDs.c

reads DAC */


#include <stdio.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay

/* Function to intiialise DAC1, connected to pin PA4     */
void DAC_Init(void) {
	/* Enable clocks */
	RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR  |= RCC_APB1ENR_DACEN;
	
	/* set PA4 and PA5 to analogue */
	GPIOA->MODER |= 3UL << (2 * 4);		
	GPIOA->PUPDR &= ~(3UL << (2 * 4));
	
	/* turn DAC channel 1 on*/
  DAC->CR |= DAC_CR_EN1;
	
}
	
/* function to write DAC1. Uses a 12 bit right-aligned number */
void write_DAC(uint32_t val) {
	DAC->DHR12R1 = val;
}


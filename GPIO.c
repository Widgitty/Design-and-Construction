#include "STM32F4xx.h"
#include "GPIO.h"
#include "Defines.h"

const unsigned long GPIO_mask[] = {1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7};
const unsigned long GPIO_mode_mask[] = {1UL << 5, 1UL << 6, 1UL << 13};

/*----------------------------------------------------------------------------
  initialize LED Pins
 *----------------------------------------------------------------------------*/
void GPIO_Init (void) {

	
  RCC->AHB1ENR  |= ((1UL <<  3) );         /* Enable GPIOD clock                */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOC->MODER    &= ~((3UL << 2*5) |
											 (3UL << 2*6) |
											 (3UL << 2*13));
	GPIOC->MODER    |=  ((1UL << 2*5) |
											 (1UL << 2*6) |
											 (1UL << 2*13));
	GPIOC->OTYPER   &= ~((1UL <<    5)|
											 (1UL <<   6) |
											 (1UL <<   13));
	GPIOC->OSPEEDR  &= ~((3UL << 2*5) |
											 (3UL << 2*6) |
											 (3UL << 2*13));
	GPIOC->OSPEEDR  |=  ((2UL << 2*5) |
											 (2UL << 2*6) |
											 (2UL << 2*13));
	GPIOC->PUPDR    &= ~((3UL << 2*5) |
											 (3UL << 2*6) |
											 (3UL << 2*13));
	GPIOC->PUPDR    |=  ((1UL << 2*5) |
											 (1UL << 2*6) |
											 (1UL << 2*13));
  GPIOE->MODER    &= ~((3UL << 2*3) |
                       (3UL << 2*4) |
                       (3UL << 2*5) |
                       (3UL << 2*6) |
                       (3UL << 2*7) );   /* PD.12..15 is output               */
  GPIOE->MODER    |=  ((1UL << 2*3) |
                       (1UL << 2*4) | 
                       (1UL << 2*5) | 
                       (1UL << 2*6) | 
                       (1UL << 2*7) ); 
  GPIOE->OTYPER   &= ~((1UL <<   3) |
                       (1UL <<   4) |
                       (1UL <<   5) |
                       (1UL <<   6) |
                       (1UL <<   7) );   /* PD.12..15 is output Push-Pull     */
  GPIOE->OSPEEDR  &= ~((3UL << 2*3) |
                       (3UL << 2*4) |
                       (3UL << 2*5) |
                       (3UL << 2*6) |
                       (3UL << 2*7) );   /* PD.12..15 is 50MHz Fast Speed     */
  GPIOE->OSPEEDR  |=  ((2UL << 2*3) |
                       (2UL << 2*4) | 
                       (2UL << 2*5) | 
                       (2UL << 2*6) | 
                       (2UL << 2*7) ); 
  GPIOE->PUPDR    &= ~((3UL << 2*3) |
                       (3UL << 2*4) |
                       (3UL << 2*5) |
                       (3UL << 2*6) |
                       (3UL << 2*7) );   /* PD.12..15 is Pull up              */
  GPIOE->PUPDR    |=  ((1UL << 2*3) |
                       (1UL << 2*4) | 
                       (1UL << 2*5) | 
                       (1UL << 2*6) | 
                       (1UL << 2*7) ); 
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void GPIO_On (unsigned int num) {

  if (num < GPIO_NUM) {
    GPIOE->BSRR = GPIO_mask[num];
  }
}

void GPIO_SetMode(int mode){
	GPIO_Off(0);
	switch(mode){
		case CURRMODE:
			//GPIO_Off(0);
			GPIOC->BSRR = GPIO_mode_mask[0];
			GPIOC->BSRR = GPIO_mode_mask[1] << 16;
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			break;
		case VOLTMODE:
			GPIOC->BSRR = GPIO_mode_mask[0] << 16;
			GPIOC->BSRR = GPIO_mode_mask[1] << 16;
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			break;
		case RESMODE:
			GPIOC->BSRR = GPIO_mode_mask[0] << 16;
			GPIOC->BSRR = GPIO_mode_mask[1];
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			GPIO_Off(0);
			break;
		case CAPMODE:
			GPIOC->BSRR = GPIO_mode_mask[0] << 16;
			GPIOC->BSRR = GPIO_mode_mask[1];
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			break;
		case INDMODE:
			GPIOC->BSRR = GPIO_mode_mask[0];
			GPIOC->BSRR = GPIO_mode_mask[1];
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			break;
		case DIODE:
			GPIO_Off(0);
			break;
		case CONTMODE:
			GPIO_On(0);
			break;
		case RMS:
			GPIOC->BSRR = GPIO_mode_mask[0] << 16;
			GPIOC->BSRR = GPIO_mode_mask[1] << 16;
			GPIOC->BSRR = GPIO_mode_mask[2];
			break;
		case FREQMODE:
			GPIOC->BSRR = GPIO_mode_mask[0];
			GPIOC->BSRR = GPIO_mode_mask[1] << 16;
			GPIOC->BSRR = GPIO_mode_mask[2] << 16;
			break;
		default:
			GPIOC->BSRR = GPIO_mode_mask[0];
			GPIOC->BSRR = GPIO_mode_mask[1];
			GPIOC->BSRR = GPIO_mode_mask[2];
			break;
	}
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void GPIO_Off (unsigned int num) {

  if (num < GPIO_NUM) {
    GPIOE->BSRR = GPIO_mask[num] << 16;
  }
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void GPIO_Out(unsigned int value) {
  int i;

  for (i = 0; i < GPIO_NUM; i++) {
    if (value & (1<<i)) {
      GPIO_On (i);
    } else {
      GPIO_Off(i);
    }
  }
}

#include "STM32F4xx.h"
#include "GPIO.h"

const unsigned long GPIO_mask[] = {1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7};

/*----------------------------------------------------------------------------
  initialize LED Pins
 *----------------------------------------------------------------------------*/
void GPIO_Init (void) {

  RCC->AHB1ENR  |= ((1UL <<  3) );         /* Enable GPIOD clock                */

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
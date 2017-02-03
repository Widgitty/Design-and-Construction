/*----------------------------------------------------------------------------
 * Name:    LCD.c
 * Purpose: low level LCD functions
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
#include "LCD.h"

const unsigned long lcd_mask[] = {1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7};
const unsigned long lcd_RS    = 1UL << 0;
const unsigned long lcd_RdnWr = 1UL << 1;
const unsigned long lcd_E     = 1UL << 2;

/*----------------------------------------------------------------------------
  initialize LCD Pins
 *----------------------------------------------------------------------------*/
void LCD_Initpins (void) {

  RCC->AHB1ENR    |=  ((1UL <<    3)  );   /* Enable GPIOD clock              */

  GPIOD->MODER    &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2) |
                       (3UL << 2* 3) |
                       (3UL << 2* 4) |
                       (3UL << 2* 5) |
                       (3UL << 2* 6) |
                       (3UL << 2* 7)  );   /* PD.0..7 is output               */
  GPIOD->MODER    |=  ((1UL << 2* 0) |
                       (1UL << 2* 1) | 
                       (1UL << 2* 2) | 
                       (1UL << 2* 3) | 
                       (1UL << 2* 4) | 
                       (1UL << 2* 5) | 
                       (1UL << 2* 6) | 
                       (1UL << 2* 7)  ); 
  GPIOD->OTYPER   &= ~((1UL <<    0) |
                       (1UL <<    1) |
                       (1UL <<    2) |
                       (1UL <<    3) |
                       (1UL <<    4) |
                       (1UL <<    5) |
                       (1UL <<    6) |
                       (1UL <<    7)  );   /* PD.0..7 is output Push-Pull     */
  GPIOD->OSPEEDR  &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2) |
                       (3UL << 2* 3) |
                       (3UL << 2* 4) |
                       (3UL << 2* 5) |
                       (3UL << 2* 6) |
                       (3UL << 2* 7)  );   /* PD.0..7 is 50MHz Fast Speed     */
  GPIOD->OSPEEDR  |=  ((2UL << 2* 0) |
                       (2UL << 2* 1) | 
                       (2UL << 2* 2) | 
                       (2UL << 2* 3) | 
                       (2UL << 2* 4) | 
                       (2UL << 2* 5) | 
                       (2UL << 2* 6) | 
                       (2UL << 2* 7)  ); 
  GPIOD->PUPDR    &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2) |
                       (3UL << 2* 3) |
                       (3UL << 2* 4) |
                       (3UL << 2* 5) |
                       (3UL << 2* 6) |
                       (3UL << 2* 7)  );   /* PD.0..7 is Pull up              */
  GPIOD->PUPDR    |=  ((1UL << 2* 0) |
                       (1UL << 2* 1) | 
                       (1UL << 2* 2) | 
                       (1UL << 2* 3) | 
                       (1UL << 2* 4) | 
                       (1UL << 2* 5) | 
                       (1UL << 2* 6) | 
                       (1UL << 2* 7)  ); 


  // PORT B LCD Control Lines
  RCC->AHB1ENR    |=  ((1UL <<    1)  );   /* Enable GPIOB clock              */

  GPIOB->MODER    &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2)  );   /* PB.0..7 is output               */
  GPIOB->MODER    |=  ((1UL << 2* 0) |
                       (1UL << 2* 1) | 
                       (1UL << 2* 2)  ); 
  GPIOB->OTYPER   &= ~((1UL <<    0) |
                       (1UL <<    1) |
                       (1UL <<    2)  );   /* PB.0..7 is output Push-Pull     */
  GPIOB->OSPEEDR  &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2)  );   /* PB.0..7 is 50MHz Fast Speed     */
  GPIOB->OSPEEDR  |=  ((2UL << 2* 0) |
                       (2UL << 2* 1) | 
                       (2UL << 2* 2)  ); 
  GPIOB->PUPDR    &= ~((3UL << 2* 0) |
                       (3UL << 2* 1) |
                       (3UL << 2* 2)  );   /* PB.0..7 is Pull up              */
  GPIOB->PUPDR    |=  ((1UL << 2* 0) |
                       (1UL << 2* 1) | 
                       (1UL << 2* 2)  ); 

  // PORT A LCD Driver IC Enable Line
  RCC->AHB1ENR    |=  ((1UL <<    0)  );   /* Enable GPIOA clock              */

  GPIOA->MODER    &= ~((3UL << 2*15)  );   /* PA.0..7 is output               */
  GPIOA->MODER    |=  ((1UL << 2*15)  ); 
  GPIOA->OTYPER   &= ~((1UL <<   15)  );   /* PA.15 is output Push-Pull     */
  GPIOA->OSPEEDR  &= ~((3UL << 2*15)  );   /* PA.15 is 50MHz Fast Speed     */
  GPIOA->OSPEEDR  |=  ((2UL << 2*15)  ); 
  GPIOA->PUPDR    &= ~((3UL << 2*15)  );   /* PA.15 is Pull up              */
  GPIOA->PUPDR    |=  ((1UL << 2*15)  ); 

}


/*----------------------------------------------------------------------------
  initialize LCD Chip
 *----------------------------------------------------------------------------*/
void LCD_Init (void) {
	
	Delay(20);
	
	LCD_Eoff();
	LCD_RSoff();
	LCD_RWoff();
	
	Delay(20);
	
	LCD_DataWrite( 0x20 );
	LCD_Clk();
	Delay(1);
	LCD_Clk();
	Delay(1);
	LCD_Clk();
	
	Delay(20);
	
	// Function Set: 4 bit mode, 1/16 duty, 5x8 font, 2 lines
	LCD_Write(0x28);
	
	// Display ON/OFF Control: ON
	LCD_Write(0x0C);
	
	// Entry Mode Set: Increment
	LCD_Write(0x06);
	
	// Clear Display
	 LCD_Clear();
  Delay(100);
}


/*----------------------------------------------------------------------------
  Function that turns on LCD E Signal
 *----------------------------------------------------------------------------*/
void LCD_Eon (void) {

    GPIOB->BSRR = lcd_E;
}

/*----------------------------------------------------------------------------
  Function that turns off LCD E Signal
 *----------------------------------------------------------------------------*/
void LCD_Eoff (void) {

    GPIOB->BSRR = lcd_E << 16;
}


/*----------------------------------------------------------------------------
  Function that turns on LCD RS Signal
 *----------------------------------------------------------------------------*/
void LCD_RSon (void) {

    GPIOB->BSRR = lcd_RS;
}

/*----------------------------------------------------------------------------
  Function that turns off LCD RS Signal
 *----------------------------------------------------------------------------*/
void LCD_RSoff (void) {

    GPIOB->BSRR = lcd_RS << 16;
}

/*----------------------------------------------------------------------------
  Function that turns on LCD RW Signal   Read notWrite
 *----------------------------------------------------------------------------*/
void LCD_RWon (void) {

    GPIOB->BSRR = lcd_RdnWr;
}

/*----------------------------------------------------------------------------
  Function that turns off LCD RW Signal   Read notWrite
 *----------------------------------------------------------------------------*/
void LCD_RWoff (void) {

    GPIOB->BSRR = lcd_RdnWr << 16;
}



/*----------------------------------------------------------------------------
  Function that outputs value to LCD Data Signals
 *----------------------------------------------------------------------------*/
void LCD_DataWrite(unsigned int value) {

	// Set Data Bits
	GPIOD->BSRR = value;
}
/*----------------------------------------------------------------------------
  Function that clears value on LCD Data Signals
 *----------------------------------------------------------------------------*/
void LCD_DataClear() {
  
	// Clear Data Bits
	GPIOD->BSRR = 0xFF << 16;
}

/*----------------------------------------------------------------------------
  Function that turns on requested LCD
 *----------------------------------------------------------------------------*/
void LCD_On (unsigned int num) {

  if (num < LCD_NUM) {
    GPIOD->BSRR = lcd_mask[num];
  }
}

/*----------------------------------------------------------------------------
  Function that turns off requested LCD
 *----------------------------------------------------------------------------*/
void LCD_Off (unsigned int num) {

  if (num < LCD_NUM) {
    GPIOD->BSRR = lcd_mask[num] << 16;
  }
}

/*----------------------------------------------------------------------------
  Function that Enables LCD driver
 *----------------------------------------------------------------------------*/
void LCD_DriverOn (void) {

    GPIOA->BSRR = 1UL << 15;
}

/*----------------------------------------------------------------------------
  Function that disables LCD driver
 *----------------------------------------------------------------------------*/
void LCD_DriverOff (void) {

    GPIOA->BSRR = 1UL << 31;
}


/*----------------------------------------------------------------------------
  Function that turns on requested LCD
 *----------------------------------------------------------------------------*/
void LCD_Clk (void) {

  LCD_Eon();
	Delay(1);
	LCD_Eoff();
}


/*----------------------------------------------------------------------------
  Function that Writes a byte to the LCD - 4 bit mode
 *----------------------------------------------------------------------------*/
void LCD_Write (unsigned int value) {

  LCD_DataClear();
	LCD_DataWrite(value & 0xF0);
	LCD_Clk();
	Delay(1);
	 
	LCD_DataClear();
	LCD_DataWrite((value<<4) & 0xF0);
	LCD_Clk();
	Delay(1);
}

/* Function that turns the display on, sets cursor on and blinking */
void LCD_cursor_on (void) {
	
	LCD_RSoff();
	LCD_Write(0x03);
	Delay(10);
}

/*----------------------------------------------------------------------------
  Function that Clears the LCD Screen
 *----------------------------------------------------------------------------*/
void LCD_Clear (void) {

  LCD_RSoff();
	LCD_Write(0x01);
	Delay(10);
}


/*----------------------------------------------------------------------------
  Function that Writes a Character to the LCD
 *----------------------------------------------------------------------------*/
void LCD_PutChar (unsigned int c) {

  LCD_RSon();
	LCD_Write(c);
	Delay(1);
}


/*----------------------------------------------------------------------------
  Function that Writes a string to the LCD
 *----------------------------------------------------------------------------*/
void LCD_PutS (const char * s) {

  LCD_RSon();
	while(*s)
		LCD_Write(*s++);
	Delay(1);
}


/*----------------------------------------------------------------------------
  Function that Sets the Cursor Postion on the LCD
 *----------------------------------------------------------------------------*/
void LCD_GotoXY (unsigned int x, unsigned int y) {

  LCD_RSoff();
	// x is 0 to 15
	// y is 0 or 1 for line
	if( y == 0 )
		LCD_Write( 0x80 | (x & 0x3F));
	else
		if( y == 1 )
			LCD_Write( 0xC0 | (x & 0x3F));

	Delay(1);
}

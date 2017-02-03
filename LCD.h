/*----------------------------------------------------------------------------
 * Name:    LCD.h
 * Purpose: low level LCD definitions
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

#ifndef __LCD_H
#define __LCD_H
#include <stdint.h>

/* LCD Definitions */
#define LCD_NUM     8                        /* Number of user LCDs          */

extern void Delay (uint32_t dlyTicks);

extern void LCD_Initpins (void);
extern void LCD_Init  (void);
extern void LCD_Eon   (void);
extern void LCD_Eoff  (void);
extern void LCD_RSon  (void);
extern void LCD_RSoff (void);
extern void LCD_RWon  (void);
extern void LCD_RWoff (void);
extern void LCD_DataWrite (unsigned int value);
extern void LCD_DataClear (void);

extern void LCD_On  (unsigned int num);
extern void LCD_Off (unsigned int num);

extern void LCD_DriverOn  (void);
extern void LCD_DriverOff (void);

extern void LCD_Clk(void);
extern void LCD_Write (unsigned int value);
extern void LCD_Clear (void);
extern void LCD_PutChar (unsigned int c);
extern void LCD_PutS (const char * s);
extern void LCD_GotoXY (unsigned int x, unsigned int y);

#endif

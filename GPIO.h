/*----------------------------------------------------------------------------
 * Name:    LED.h
 * Purpose: low level LED definitions
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

#ifndef __GPIO_H
#define __GPIO_H

/* LED Definitions */
#define GPIO_NUM     5                        /* Number of user LEDs          */

extern void GPIO_Init(void);
extern void GPIO_On  (unsigned int num);
extern void GPIO_Off (unsigned int num);
extern void GPIO_Out (unsigned int value);

#endif

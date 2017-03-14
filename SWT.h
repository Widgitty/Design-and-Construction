/*----------------------------------------------------------------------------
 * Name:    SWT.h
 * Purpose: low level SWT input switches definitions
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

#ifndef __SWT_H
#define __SWT_H
#include <stdint.h>

/* SWT Definitions */
#define SWT_NUM     8                        /* Number of user Switches       */

extern void SWT_Init(void);
extern uint32_t SWT_Get (void);
extern int SWT_Check (unsigned int num);
uint32_t SWT_Debounce(void);
uint32_t getMode(void);

#endif

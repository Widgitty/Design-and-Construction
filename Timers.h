#ifndef __TIMERS_H
#define __TIMERS_H

#include "STM32F4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "Timers.h"

extern TIM_HandleTypeDef timer_Instance_1;
extern TIM_HandleTypeDef timer_Instance_2;
extern void Timer_Init(void);
extern void Interrupt_Init(void);
extern void EXTI4_IRQHandler(void);
extern int capacitorState;

#endif

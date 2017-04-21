#ifndef __TIMERS_H
#define __TIMERS_H

#include "STM32F4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "Timers.h"

extern TIM_HandleTypeDef timer_Instance_1;
extern TIM_HandleTypeDef timer_Instance_2;
extern TIM_HandleTypeDef timer_Instance_3;
extern TIM_HandleTypeDef timer_Instance_4;
extern TIM_HandleTypeDef timer_Instance_5;
extern void Timer_Init(void);
extern void Interrupt_Init(void);
extern void EXTI4_IRQHandler(void);
extern int capacitorState;
extern int inductanceState;
extern int frequencyState;

#endif

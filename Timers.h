#ifndef __TIMERS_H
#define __TIMERS_H

#include "STM32F4xx_hal.h"
#include "stm32f4xx_hal_tim.h"

extern TIM_HandleTypeDef timer_Instance;
extern void Init_Timer(void);

#endif
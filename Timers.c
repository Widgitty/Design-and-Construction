#include "Timers.h"
#include "STM32F4xx_hal.h"
#include "stm32f4xx_hal_tim.h"

static TIM_HandleTypeDef timer_Instance = { .Instance = TIM3};

void Init_Timer(void) {
	__TIM3_CLK_ENABLE();
	timer_Instance.Init.Prescaler = 1000000;
	timer_Instance.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance.Init.Period = 1000;
	timer_Instance.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance);
	HAL_TIM_Base_Start(&timer_Instance);
}
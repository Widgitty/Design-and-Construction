#include "Timers.h"

static TIM_HandleTypeDef timer_Instance = { .Instance = TIM3};


// Initialises a simple timer. Timing has to be set up correctly
void Init_Timer(void) {
	__TIM3_CLK_ENABLE();
	timer_Instance.Init.Prescaler = 1000000;
	timer_Instance.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance.Init.Period = 1000;
	timer_Instance.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance);
	HAL_TIM_Base_Start(&timer_Instance);
	HAL_TIM_Base_Start_IT(&timer_Instance);
}

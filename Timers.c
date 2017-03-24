#include "Timers.h"
#include "LED.h"
#include "stm32f4xx_hal_rcc.h"

static TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
static TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};

// Initialises a simple timer. Timing has to be set up correctly
void Timer_Init(void) {
	uint32_t clockFreq = HAL_RCC_GetPCLK1Freq();
   // clockFreq/5000 is about right 
	//TODO should probably check timer speed for accuracy
	
	__TIM2_CLK_ENABLE();
	timer_Instance_1.Init.Prescaler = clockFreq / 5000;
	timer_Instance_1.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_1.Init.Period = 50000;
	timer_Instance_1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_1.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_1);
	HAL_TIM_Base_Start(&timer_Instance_1);
	HAL_TIM_Base_Start_IT(&timer_Instance_1);
	
	
	__TIM3_CLK_ENABLE();
	timer_Instance_2.Init.Prescaler = clockFreq / 5000;
	timer_Instance_2.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_2.Init.Period = 50000;
	timer_Instance_2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_2.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_2);
	HAL_TIM_Base_Start(&timer_Instance_2);
	HAL_TIM_Base_Start_IT(&timer_Instance_2);
}

void Interrupt_Init(){
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	RCC->AHB1ENR    |=  RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER    &= ~((3UL << 2* 4));
  GPIOB->OSPEEDR  &= ~((3UL << 2* 4));
  GPIOB->PUPDR    &= ~((3UL << 2* 4));
	GPIOB->PUPDR    |= 	((2UL << 2* 4));
	
	SYSCFG->EXTICR[1] = SYSCFG_EXTICR2_EXTI4_PB;
	EXTI->RTSR |= 1UL << 4;
	EXTI->IMR |= 1UL << 4;
	NVIC_EnableIRQ(EXTI4_IRQn);
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);
}

//timer interrupt to reset the capacitance measurements if a timeout happens (5 seconds)
void TIM2_IRQHandler(void){
	capacitorState = 0;
	TIM2->SR &= ~TIM_SR_UIF;
}

// external interrupt to stop the timer:
void EXTI4_IRQHandler(void){
	
	EXTI->PR |= 1 << 4;
	HAL_TIM_Base_Stop(&timer_Instance_1);
}

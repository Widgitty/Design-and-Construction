#include "Timers.h"
#include "LED.h"
#include "stm32f4xx_hal_rcc.h"

static TIM_HandleTypeDef timer_Instance = { .Instance = TIM2};


// Initialises a simple timer. Timing has to be set up correctly
void Timer_Init(void) {
	
	uint32_t clockFreq = HAL_RCC_GetPCLK1Freq();
   // clockFreq/5000 is about right 
	//TODO should probably check timer speed for accuracy
	
	__TIM2_CLK_ENABLE();
	timer_Instance.Init.Prescaler = clockFreq / 5000;
	timer_Instance.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance.Init.Period = 50000;
	timer_Instance.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance);
	HAL_TIM_Base_Start(&timer_Instance);
	HAL_TIM_Base_Start_IT(&timer_Instance);
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
}

void EXTI4_IRQHandler(void){
	EXTI->PR |= 1 << 4;
	LED_On(7);
	LED_On(6);
	LED_On(5);
	LED_On(4);
	HAL_TIM_Base_Stop(&timer_Instance);
}

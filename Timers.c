#include "Timers.h"
#include "GPIO.h"
#include "stm32f4xx_hal_rcc.h"
#include "Defines.h"
#include "Calculations.h"

static TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
static TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};
static TIM_HandleTypeDef timer_Instance_3 = { .Instance = TIM4};
static TIM_HandleTypeDef timer_Instance_4 = { .Instance = TIM5};

int on = 0;

// Initialises a simple timer. Timing has to be set up correctly
void Timer_Init(void) {
	uint32_t clockFreq = HAL_RCC_GetPCLK1Freq();
   // clockFreq/5000 is about right 
	//TODO should probably check timer speed for accuracy
	
	
	// timer used for frequency measurements
	__TIM2_CLK_ENABLE();
	timer_Instance_1.Init.Prescaler = clockFreq / PRESCALERM;
	timer_Instance_1.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_1.Init.Period = 40000;
	timer_Instance_1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_1.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_1);
	//HAL_TIM_Base_Start(&timer_Instance_1);
	//HAL_TIM_Base_Start_IT(&timer_Instance_1);
	
	
	// timer used for averaging
	__TIM3_CLK_ENABLE();
	timer_Instance_2.Init.Prescaler = clockFreq / PRESCALERL;
	timer_Instance_2.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_2.Init.Period = PERIODL;
	timer_Instance_2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_2.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_2);
	//HAL_TIM_Base_Start(&timer_Instance_2);
	//HAL_TIM_Base_Start_IT(&timer_Instance_2);
	
	// timer used for pulse generation for inductance measurements
	__TIM4_CLK_ENABLE();
	timer_Instance_3.Init.Prescaler = clockFreq/500000;
	timer_Instance_3.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_3.Init.Period = 50000;
	timer_Instance_3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_3.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_3);
	
		// used for capacitance and inductance measurements
	__TIM5_CLK_ENABLE();
	timer_Instance_4.Init.Prescaler = clockFreq / PRESCALERS;
	timer_Instance_4.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer_Instance_4.Init.Period = PERIODL;
	timer_Instance_4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer_Instance_4.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&timer_Instance_4);
	//HAL_TIM_Base_Start(&timer_Instance_4);
	//HAL_TIM_Base_Start_IT(&timer_Instance_4);	
}

void Interrupt_Init(){
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	RCC->AHB1ENR    |=  RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER    &= ~((3UL << 2* 4));
  GPIOB->OSPEEDR  &= ~((3UL << 2* 4));
  GPIOB->PUPDR    &= ~((3UL << 2* 4));
	GPIOB->PUPDR    |= 	((2UL << 2* 4));
	GPIOB->MODER    &= ~((3UL << 2* 5));
  GPIOB->OSPEEDR  &= ~((3UL << 2* 5));
  GPIOB->PUPDR    &= ~((3UL << 2* 5));
	GPIOB->PUPDR    |= 	((2UL << 2* 5));
	
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
	EXTI->RTSR |= 1UL << 4;
	EXTI->IMR |= 1UL << 4;
	NVIC_EnableIRQ(EXTI4_IRQn);
	
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PB;
	EXTI->RTSR |= 1UL << 5;
	//EXTI->FTSR |= 1UL << 5;
	EXTI->IMR |= 1UL << 5;
	
	
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);
	
	TIM5->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM5_IRQn);
	
	TIM4->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM4_IRQn);
}

// interrupt handler for timers
void TIM4_IRQHandler(void){
	TIM4->SR &= ~TIM_SR_UIF;
	if(on == 0)
	{
		GPIO_On(3);
		on = 1;
	}
	else
	{
		on = 0;
		GPIO_Off(3);
	}
}

//timer interrupt to reset the capacitance measurements if a timeout happens (5 seconds)
void TIM2_IRQHandler(void){
	TIM2->SR &= ~TIM_SR_UIF;
	setOutput();
}
void TIM5_IRQHandler(void){
	capacitorState = 0;
	inductanceState = 2;
	TIM5->SR &= ~TIM_SR_UIF;
}

// external interrupt to stop the timer:
void EXTI4_IRQHandler(void){
	
	EXTI->PR |= 1 << 4;
	HAL_TIM_Base_Stop(&timer_Instance_4);
}

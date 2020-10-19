#include "timer.h"

void timer2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	TIM2->PSC = 35999;
	TIM2->ARR = 59999;
	TIM2->CNT = 0;
	TIM2->EGR = TIM_EGR_UG;
	
	TIM2->DIER |= TIM_DIER_UIE;
	
	TIM2->CR1 = TIM_CR1_CEN;
	
	NVIC_SetPriority(TIM2_IRQn, 1);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void timer3_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	
	TIM3->PSC = 35999;
	TIM3->ARR = 999;
	TIM3->CNT = 0;
	TIM3->EGR = TIM_EGR_UG;
	
	TIM3->DIER |= TIM_DIER_UIE;
	
	TIM3->CR1 = TIM_CR1_CEN;
	
	NVIC_SetPriority(TIM3_IRQn, 2);
	NVIC_EnableIRQ(TIM3_IRQn);
}

void timer4_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	
	TIM4->PSC = 35999;
	TIM4->ARR = 29999;
	TIM4->CNT = 0;
	TIM4->EGR = TIM_EGR_UG;
	
	TIM4->DIER |= TIM_DIER_UIE;
	
	TIM4->CR1 = TIM_CR1_CEN;
	
	NVIC_SetPriority(TIM4_IRQn, 3);
	NVIC_EnableIRQ(TIM4_IRQn);
}

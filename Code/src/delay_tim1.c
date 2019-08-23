#include "delay_tim1.h"

uint32_t prescaler_ms;
uint32_t prescaler_us;

void delay_us(uint16_t value)
{
	TIM1->PSC = prescaler_us;
	TIM1->ARR = value;
	TIM1->CNT = 0;
	TIM1->CR1 = TIM_CR1_CEN;
	while((TIM1->SR & TIM_SR_UIF) == 0){}
	TIM1->SR &= ~TIM_SR_UIF;
}

void delay_ms(uint32_t value)
{
	TIM1->PSC = prescaler_ms;
	TIM1->ARR = value;
	TIM1->CNT = 0;
	TIM1->CR1 = TIM_CR1_CEN;
	while((TIM1->SR & TIM_SR_UIF) == 0){}
	TIM1->SR &= ~TIM_SR_UIF;
}

void delay_tim1_init(void)
{
	prescaler_ms = SystemCoreClock / 1000;
	prescaler_us = SystemCoreClock / 1000000;
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
}

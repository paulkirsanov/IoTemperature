#include "exti.h"

void exti_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRL &= ~GPIO_CRL_CNF0;
	GPIOB->CRL &= ~GPIO_CRL_MODE0;
	GPIOB->CRL |= GPIO_CRL_CNF0_1;
	GPIOB->CRL |= GPIO_CRL_MODE0_1;
//	GPIOB->ODR |= GPIO_ODR_ODR0;
	
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PB;			//
	
	EXTI->RTSR |= EXTI_RTSR_TR0;										//Rising trigger event configuration bit of line 0
//	EXTI->FTSR |= EXTI_FTSR_TR0;										//Falling trigger event configuration bit of line 0
	EXTI->PR = EXTI_PR_PR0;													//Pending bit for line 0
	EXTI->IMR |= EXTI_IMR_MR0;											//Interrupt request
	NVIC_EnableIRQ(EXTI0_IRQn);											//Enable IRQ
}

void exti17_init(void)
{
	EXTI->PR |= EXTI_PR_PR17;												//Pending bit for line 17
	EXTI->IMR |= EXTI_IMR_MR17;											//Interrupt request
	EXTI->EMR &= ~EXTI_EMR_MR17;
	
	EXTI->RTSR |= EXTI_RTSR_TR17;										//Rising trigger event configuration bit of line 17
	EXTI->FTSR &= ~EXTI_FTSR_TR17;									//falling trigger disabled
	
	NVIC_SetPriority(RTCAlarm_IRQn, 1);
  NVIC_EnableIRQ(RTCAlarm_IRQn);
}

#include "main.h"

ring_buffer_t ring_rx;

void rcc_init(void);

int main(void)
{
	rcc_init();
	usart2_init();
	led_init();
	timer3_init();
	delay_tim4_init();
	
	ring_init(&ring_rx, 256);

	while(1)
	{
		
	}
}

void rcc_init(void)
{
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);												//Enable HSE
	while(!(RCC->CR & RCC_CR_HSERDY));													//Ready start HSE
	
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;					//Clock flash memory
	
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;														//AHB = SYSCLK/1
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;														//APB1 = HCLK/4
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV4;														//APB2 = HCLK/4
	
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;															//clear PLLMULL bits
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;															//clear PLLSRC bits
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;														//clear PLLXPRE bits
	
	RCC->CFGR |= RCC_CFGR_PLLSRC;																//source HSE
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;													//sorce HSE/1 = 8MHz
	RCC->CFGR |= RCC_CFGR_PLLMULL9;															//PLL x9: clock = 8MHz * 9 = 72 MHz
	
	RCC->CR |= RCC_CR_PLLON;																		//enable PLL
	while(!(RCC->CR & RCC_CR_PLLRDY));													//wait till PLL is ready
	
	RCC->CFGR &= ~RCC_CFGR_SW;																	//clear SW bits
	RCC->CFGR |= RCC_CFGR_SW_PLL;																//select surce SYSCLK = PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1){}				//wait till PLL is used
}

void USART2_IRQHandler(void)
{
	if(USART2->SR & USART_SR_RXNE)
	{
		USART2->SR &= ~USART_SR_RXNE;
		
		ring_put(USART2->DR, &ring_rx);
	}
}

void TIM3_IRQHandler(void)
{
	if(TIM3->SR & TIM_SR_UIF)
	{
		TIM3->SR &= ~TIM_SR_UIF;
		
		if(GPIOC->ODR & GPIO_ODR_ODR13){
			GPIOC->BSRR |= GPIO_BSRR_BR13;
		} else {
			GPIOC->BSRR |= GPIO_BSRR_BS13;
		}
	}
}

#include "lowpower.h"

void sleep_mode(void)
{
	SCB->SCR |= SCB_SCR_SLEEPONEXIT;
}

void stop_mode(void)
{
	PWR->CR |= PWR_CR_PVDE;																						//Power Voltage Detector Enable
	PWR->CR |= PWR_CR_LPDS;																						//Low-Power Deepsleep
	PWR->CR &= ~PWR_CR_PDDS;																					//Clear Power Down Deepsleep
	SCB->SCR |= SCB_SCR_SLEEPDEEP;																		//Sleep deep bit
	PWR->CSR &= ~PWR_CSR_WUF;																					//Wakeup Flag
}

void standby_mode(void)
{
	PWR->CR &= ~PWR_CR_LPDS;																					//Low-Power Deepsleep
	PWR->CR |= PWR_CR_PDDS;																						//Power Down Deepsleep
	SCB->SCR |= SCB_SCR_SLEEPDEEP;																		//Sleep deep bit
	PWR->CSR &= ~PWR_CSR_WUF;																					//Wakeup Flag
}

void reset_pins(void)
{
	RCC->APB2ENR 	|= RCC_APB2ENR_IOPAEN;
	
	GPIOA->CRL &= ~( GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 | GPIO_CRL_CNF3 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7 );
	GPIOA->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11 | GPIO_CRH_CNF12 | GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15 );
	
	GPIOA->CRL &= ~( GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7 );
	GPIOA->CRH &= ~( GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15 );
	
	RCC->APB2ENR 	|= RCC_APB2ENR_IOPBEN;
	
	GPIOB->CRL &= ~( GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 | GPIO_CRL_CNF3 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7 );
	GPIOB->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11 | GPIO_CRH_CNF12 | GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15 );
	
	GPIOB->CRL &= ~( GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7 );
	GPIOB->CRH &= ~( GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15 );

	RCC->APB2ENR 	|= RCC_APB2ENR_IOPCEN;
	
	GPIOC->CRL &= ~( GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 | GPIO_CRL_CNF3 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7 );
	GPIOC->CRH &= ~( GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11 | GPIO_CRH_CNF12 | GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15 );
		
	GPIOC->CRL &= ~( GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7 );
	GPIOC->CRH &= ~( GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15 );
}
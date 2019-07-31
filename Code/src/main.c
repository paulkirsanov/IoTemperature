#include "main.h"

float temperature;
unsigned char buffer_ROM[8];
char text_temperature[100] = {0};
char text[100] = {0};

uint8_t count_packet = 0;
uint8_t count_error = 0;

esp8266_status current_status = { ESP8266_STATUS_NOWIFI, {0} };
esp8266_client tp_client = { "184.106.153.149", "80", ESP8266_TCP};
esp8266_ap ap_client = { "My_WIFI", "pass0000"};

void rcc_init(void);

int main(void)
{
	rcc_init();
	usart1_init();
	usart2_init();
	led_init();
	
	timer3_init();
	delay_tim1_init();
	
	if(OneWire_Init() == 0)
	{
		if(OneWire_Read_ROM(buffer_ROM))
		{
			sprintf(text, "DS18B20 ROM %02x%02x%02x%02x%02x%02x%02x%02x \r\n", buffer_ROM[7], buffer_ROM[6], buffer_ROM[5], buffer_ROM[4], buffer_ROM[3], buffer_ROM[2], buffer_ROM[1], buffer_ROM[0]);
			usart_send_string(USART2, text);
		}
	}
	
	if(esp8266Begin())
		usart_send_string(USART2, "Init success\r\n");
	
	while(!esp8266Connect(&ap_client, ESP8266_MODE_STA))
	{
		usartSendArrar(USART2, (uint8_t *)"No connection\r\n");
		delay_ms(5000);
	}
	usartSendArrar(USART2, (uint8_t *)"Connect success\r\n");
	
	timer2_init();

	while(1)
	{
		/*if(esp8266TcpStatus(&current_status))
		{
			if(current_status.stat == ESP8266_STATUS_NOWIFI) {
				usartSendArrar(USART2, (uint8_t *)"ESP8266 STATUS NOWIFI\r\n");
			} else {
				esp8266TcpSend_packet(&tp_client, text_temperature);
			}
		}
		else
		{
			usartSendArrar(USART2, (uint8_t *)"Error\r\n");
		}
		
		delay_ms(9999);*/
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
		
	}
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		
		/*if(OneWire_Init() == 0)
		{
			temperature = OneWire_Print(buffer_ROM);
			sprintf(text_temperature, "%0.1f", temperature);
			usart_send_string(USART2, text_temperature);
		}*/
		
		
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

#include "main.h"

float temperature;
unsigned char buffer_ROM[8];
char text_temperature[5] = {0};
char text_value[5] = {0};
char text[100] = {0};

RTC_Time Time, Alarm;

esp8266_status current_status = { ESP8266_STATUS_NOWIFI, {0} };
buf packet1[6] = { { (uint8_t *)"GET /update?api_key=UHFL1R04OC12Y812&field1=", 44 }, { (uint8_t *)" HTTP/1.1\r\n", 11 }, { (uint8_t *)"Host: api.thingspeak.com\r\n", 26 }, { (uint8_t *)"Accept: */*\r\n", 13 }, { (uint8_t *)"User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n", 67 }, { (uint8_t *)"\r\n", 2 } };

	void rcc_init(void);
	void initializationTask(void);
	void measurerTask(void);
	void gpio_init(void);

int main(void)
{
	rcc_init();
	rtc_init();
	
	Time.hour = 12;
	Time.minutes = 00;
	Time.seconds = 00;
	
	Alarm.hour = 00;
	Alarm.minutes = 05;
	Alarm.seconds = 00;
	
	RTC_SetCounter(&Time);
	RTC_SetAlarm(&Alarm);
	
	usart1_init();
	usart2_init();
	led_init();
	gpio_init();
	delay_tim1_init();
	initializationTask();
	exti17_init();
//	sleep_mode();
	
	while(1)
	{
//		__WFI();
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

void measurerTask(void)
{
	if(OneWire_Init() == 0)
		{
			temperature = OneWire_Print(buffer_ROM);
			sprintf(text_temperature, "%0.1f", temperature);
			
			while(!esp8266Connect(&ap_client, ESP8266_MODE_STA))
			{
				usartSendArrar(USART2, (uint8_t *)"no connection\r\n");
				delay_ms(1000);
			}
			
			if(esp8266TcpStatus(&current_status))
			{
				if(current_status.stat == ESP8266_STATUS_NOWIFI)
				{
					usartSendArrar(USART2, (uint8_t *)"esp8266 status no wi-fi\r\n");
				}
				else if(current_status.stat == ESP8266_STATUS_CONNECTED)
				{
					usartSendArrar(USART2, (uint8_t *)"esp8266 status connected\r\n");
					esp8266TcpSend_packet(&tp_client, text_temperature, packet1);
				}
				else if(current_status.stat == ESP8266_STATUS_DISCONNECTED)
				{
					usartSendArrar(USART2, (uint8_t *)"esp8266 status disconnected\r\n");
				}
				else if(current_status.stat == ESP8266_STATUS_GOTIP)
				{
					usartSendArrar(USART2, (uint8_t *)"esp8266 status got IP\r\n");
					esp8266TcpSend_packet(&tp_client, text_temperature, packet1);
				}
				
				esp8266Disconnect();
			} else {
				usartSendArrar(USART2, (uint8_t *)"error vMeasurer\r\n");
			}
		}
}

void initializationTask(void)
{
	if(OneWire_Init() == 0)
	{
		if(!OneWire_Read_ROM(buffer_ROM))
			usart_send_string(USART2, "ds18b20 failed read ROM\r\n");
	}
	else
		usart_send_string(USART2, "ds18b20 failed initialization\r\n");
	
	if(esp8266SleepMode(ESP8266_SLEEP_DISABLE))
			usart_send_string(USART2, "esp8266 sleep mode is disable\r\n");
	
	GPIOB->BSRR |= GPIO_BSRR_BR0;
	
	while(!esp8266Begin())
		usart_send_string(USART2, "esp8266 failed initialization\r\n");
	
	while(!esp8266WakeUpGPIO());
	usart_send_string(USART2, "esp8266 WakeUpGPIO initialization\r\n");
	
	usart_send_string(USART2, "esp8266 successful initialization\r\n");
	
	GPIOB->BSRR |= GPIO_BSRR_BS0;
}

void RTCAlarm_IRQHandler(void)
{
	if(RTC->CRL & RTC_CRL_ALRF)
	{
		RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;
		
		RTC_SetAlarm(&Alarm);
		GPIOB->BSRR = GPIO_BSRR_BR0;
		measurerTask();
		while(!esp8266SleepMode(ESP8266_SLEEP_LIGHT))
			usart_send_string(USART2, "esp8266 error sleep\r\n");
		GPIOB->BSRR = GPIO_BSRR_BS0;
		
		RTC->CRL = RTC_CRL_ALRF;																			// сбросить флаг
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		EXTI->PR = EXTI_PR_PR17;
	}
}

void gpio_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRL &= ~GPIO_CRL_CNF0;
	GPIOB->CRL |= GPIO_CRL_MODE0_0;
	
	GPIOB->BSRR |= GPIO_BSRR_BS0;
}

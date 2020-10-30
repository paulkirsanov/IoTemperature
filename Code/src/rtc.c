#include "rtc.h"

extern RTC_Time Alarm;

unsigned char rtc_init(void)
{	
	RCC->APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);					//Включить тактирование PWR и Backup
	PWR->CR |= PWR_CR_DBP;																						//Разрешить доступ к Backup области
	
	if((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
	{
		RCC->BDCR |= RCC_BDCR_BDRST;																		//Выполнить сброс области резервных данных
		RCC->BDCR &= ~RCC_BDCR_BDRST;
	
		RCC->BDCR |= RCC_BDCR_LSEON;																		//Включить LSE
		while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0){}										//Дождаться включения
		
		RCC->BDCR &= ~RCC_BDCR_RTCSEL;
		RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;																//Выбрать LSE источник (кварц 32768) и подать тактирование
		RCC->BDCR |= RCC_BDCR_RTCEN;
			
		BKP->RTCCR |= 3;																								//калибровка RTC
		while((RTC->CRL & RTC_CRL_RTOFF) == 0);													//проверить закончены ли изменения регистров RTC
		
		RTC->CRL |= RTC_CRL_CNF;																				//Разрешить Запись в регистры RTC
		while((RTC->CRL & RTC_CRL_RTOFF) == 0);													//проверить закончены ли изменения регистров RTC
		RTC->PRLL = 0x7FFF;																							//Настроить делитель на 32768 (32767+1)
//		BKP->RTCCR &= ~BKP_RTCCR_CCO;																		//Выключение вывода Tamper
//		BKP->CR &= ~BKP_CR_TPE;																					//
		while((RTC->CRL & RTC_CRL_RTOFF) == 0);													//проверить закончены ли изменения регистров RTC
		RTC->PRLH = 0;
		
		// Clear alarm
		while((RTC->CRL & RTC_CRL_RTOFF) == 0);
		RTC->ALRH = 0;
		while((RTC->CRL & RTC_CRL_RTOFF) == 0);
		RTC->ALRL = 0;
		
		RTC->CRL &= ~RTC_CRL_CNF;																				//Запретить запись в регистры RTC
	//	while (!(RTC->CRL & RTC_CRL_RTOFF)){}													//Дождаться окончания записи

		RTC->CRL &= (uint16_t)~RTC_CRL_RSF;															//Синхронизировать RTC
		while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF){}								//Дождаться синхронизации

		PWR->CR &= ~PWR_CR_DBP;
			
		return 1;
	}
	return 0;
}

void RTC_SetAlarm(const RTC_Time *alarm)														//Записать новое значение будильника
{
	uint32_t value;
	
	value = (uint32_t)alarm->hour * 3600;															//Преобразовать часы
	value += (uint32_t)alarm->minutes * 60;														//Преобразовать минуты
	value += alarm->seconds - 1;																			//Преобразовать секунды
	
	value += RTC->CNTL;																								//Прибавить к текущему значению
	value += (RTC->CNTH << 16);																				//Прибавить к текущему значению
	
	PWR->CR |= PWR_CR_DBP;
	while((RTC->CRL & RTC_CRL_RTOFF) == 0);
	RTC->CRL |= RTC_CRL_CNF;
	
	RTC->CRH &= ~RTC_CRH_ALRIE;
	RTC->CRL &= ~RTC_CRL_ALRF;
		
	while((RTC->CRL & RTC_CRL_RTOFF) == 0);
	RTC->ALRH = value >> 16;
	
	while((RTC->CRL & RTC_CRL_RTOFF) == 0);
	RTC->ALRL = value & 0xFFFF;
	
	RTC->CRH |= RTC_CRH_ALRIE;
	
	RTC->CRL &= ~RTC_CRL_CNF;																					//Запретить запись в регистры RTC
	while((RTC->CRL & RTC_CRL_RTOFF) == 0);
	
//	PWR->CR &= ~PWR_CR_DBP;
}

RTC_Time *RTC_GetCounter(void)																			//Получить значение счетчика
{
	uint32_t cnt = (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
	RTC_Time *time;
	
	time->seconds = cnt % 60;																					//Сохранить секунды
	cnt /= 60;																												//Вычислить минуты
	time->minutes = cnt % 60;																					//Сохранить минуты
	cnt /= 60;																												//Вычислить часы
	time->hour = cnt % 24;																						//Сохранить часы
	
	return time;
}

void RTC_SetCounter(const RTC_Time *time)														//Записать новое значение счетчика
{
	uint32_t value;
	value = (uint32_t)time->hour * 3600;															//Преобразовать часы
	value += (uint32_t)time->minutes * 60;														//Преобразовать минуты
	value += time->seconds;																						//Преобразовать секунды
	
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;						//включить тактирование PWR и Backup
  PWR->CR |= PWR_CR_DBP;																						//разрешить доступ к Backup области
  while (!(RTC->CRL & RTC_CRL_RTOFF));															//проверить закончены ли изменения регистров RTC
  RTC->CRL |= RTC_CRL_CNF;																					//Включить режим конфигурирования
  RTC->CNTH = value >> 16;																					//записать новое значение счетного регистра
  RTC->CNTL = value;
  RTC->CRL &= ~RTC_CRL_CNF;																					//Запретить запись в регистры RTC
  while (!(RTC->CRL & RTC_CRL_RTOFF));															//Дождаться окончания записи
  PWR->CR &= ~PWR_CR_DBP;																						//запретить доступ к Backup области
}

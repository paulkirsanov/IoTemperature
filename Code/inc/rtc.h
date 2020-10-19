#ifndef RTC_H_
#define RTC_H_

#include "stm32f10x.h"
#include "usart.h"


typedef struct
{
	unsigned char hour;
	unsigned char minutes;
	unsigned char seconds;
} RTC_Time;

unsigned char rtc_init(void);
void RTC_SetAlarm(const RTC_Time *alarm);
RTC_Time *RTC_GetCounter(void);
void RTC_SetCounter(const RTC_Time *time);

#endif

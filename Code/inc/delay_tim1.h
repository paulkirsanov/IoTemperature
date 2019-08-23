#ifndef __delay_tim1_h
#define __delay_tim1_h

#define DELAY_LED		100

#include "stm32f10x.h"

void delay_ms(uint32_t value);
void delay_us(uint16_t value);
void delay_tim1_init(void);

#endif

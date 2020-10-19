#ifndef LOWPOWER_H_
#define LOWPOWER_H_

#include "stm32f10x.h"

void sleep_mode(void);
void stop_mode(void);
void standby_mode(void);
void reset_pins(void);

#endif

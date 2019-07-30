
#ifndef DS18B20_H
#define DS18B20_H

#include "stm32f10x.h"
#include "delay_tim1.h"
#include "common.h"

#define ONEWIRE_LOW()									GPIOB->BSRR |= GPIO_BSRR_BR8
#define ONEWIRE_HIGH()								GPIOB->BSRR |= GPIO_BSRR_BS8
#define ONEWIRE_INPUT_PORT()					GPIOB->IDR & GPIO_IDR_IDR8

#define ONEWIRE_CMD_RSCRATCHPAD				0xBE
#define ONEWIRE_CMD_WSCRATCHPAD				0x4E
#define ONEWIRE_CMD_CONVERTTEMP				0x44
#define ONEWIRE_CMD_CPYSCRATCHPAD			0x48
#define ONEWIRE_CMD_RECEEPROM					0xB8
#define ONEWIRE_CMD_RPWRSUPPLY				0xB4
#define ONEWIRE_CMD_SEARCHROM					0xF0
#define ONEWIRE_CMD_READROM						0x33
#define ONEWIRE_CMD_MATCHROM					0x55
#define ONEWIRE_CMD_SKIPROM						0xCC

#define ONEWIRE_FAMILY_CODE						0x28

#define DS18B20_DECIMAL_STEPS_12BIT		0.0625
#define DS18B20_DECIMAL_STEPS_11BIT		0.125
#define DS18B20_DECIMAL_STEPS_10BIT		0.25
#define DS18B20_DECIMAL_STEPS_9BIT		0.5

unsigned char OneWire_Init(void);
bool OneWire_Read_ROM(unsigned char *buffer_ROM);
float OneWire_Print(unsigned char *buffer_ROM);

#endif

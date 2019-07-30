
#include "ds18b20.h"

volatile char OK_Flag;

void SetPinAsInput(void)
{
	GPIOB->CRH &= ~GPIO_CRH_CNF8;
	GPIOB->CRH |= GPIO_CRH_CNF8_0;
	GPIOB->CRH &= ~GPIO_CRH_MODE8;
}

void SetPinAsOutput(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRH &= ~GPIO_CRH_CNF8_0;
	GPIOB->CRH |= GPIO_CRH_CNF8_1;
	GPIOB->CRH |= GPIO_CRH_MODE8;
}

unsigned char OneWire_Init(void)
{
	ONEWIRE_LOW();
	SetPinAsOutput();
	delay_us(485);
	SetPinAsInput();
	delay_us(65);
	OK_Flag = GPIOB->IDR & GPIO_IDR_IDR8;
	delay_us(500);
	return OK_Flag;
}

unsigned char OneWire_Read(void)
{
	unsigned char i;
	unsigned char data = 0;
	for (i = 0; i < 8; i++)
	{
		SetPinAsOutput();
		delay_us(2);
		SetPinAsInput();
		delay_us(4);
		data = data >> 1;
		if (ONEWIRE_INPUT_PORT())
		{
			data |= 0x80;
		}
		delay_us(62);
	}
	return data;
}

void OneWire_Write(unsigned char data)
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		SetPinAsOutput();
		delay_us(2);
		if (data & 0x01)
		{
			SetPinAsInput();
		}
		else
		{
			SetPinAsOutput();
		}
		data = data >> 1;
		delay_us(62);
		SetPinAsInput();
		delay_us(2);
	}
}

bool OneWire_Read_ROM(unsigned char *buffer_ROM)
{
	uint8_t i;
	OneWire_Init();
	OneWire_Write(ONEWIRE_CMD_READROM);
	for (i = 0; i < 8; i++)
	{
		buffer_ROM[i] = OneWire_Read();
	}
	return TRUE;
}

bool OneWire_Match_ROM(unsigned char *buffer_ROM)
{
	uint8_t i;
	OneWire_Init();
	OneWire_Write(ONEWIRE_CMD_MATCHROM);
	for (i = 0; i < 8; i++)
	{
		OneWire_Write(buffer_ROM[i]);
	}
	return TRUE;
}

bool OneWire_Search_ROM(void)
{
	OneWire_Init();
	OneWire_Write(ONEWIRE_CMD_SEARCHROM);
	
	return TRUE;
}

float OneWire_Print(unsigned char *buffer_ROM)
{
	float temperature;
	uint8_t digit, decimal;
	uint8_t data[9] = {0};
	uint8_t i;
	
	OneWire_Init();
	OneWire_Match_ROM(buffer_ROM);
	OneWire_Write(ONEWIRE_CMD_CONVERTTEMP);
	OneWire_Init();
	OneWire_Match_ROM(buffer_ROM);
	OneWire_Write(ONEWIRE_CMD_RSCRATCHPAD);
	
	for(i = 0; i < 9; i++)
	{
		data[i] = OneWire_Read();
	}
	
	digit = ((data[1] << 4) & 0x70)|(data[0] >> 4);
	decimal = data[0] & 0x0F;
	decimal *= 6;
	
	if(data[1] > 0xFB) digit = 127 - digit;
	if(decimal < 100) temperature = digit + ((float)decimal/100);
	else temperature = digit + ((float)decimal/1000);
	return temperature;
}

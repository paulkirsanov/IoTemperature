#ifndef __usart_h
#define __usart_h

#include "stm32f10x.h"
#include <stdlib.h>

typedef struct{
	uint8_t *buffer;
	uint16_t idxIn;
	uint16_t idxOut;
	uint16_t size;
} ring_buffer_t;

void usart1_init(void);
void usart2_init(void);
void usart_send_string(char* str);
void usart_send(char data);

void ring_put(uint8_t symbol, ring_buffer_t *buf);
uint8_t ring_pop(ring_buffer_t *buf);
uint16_t ring_get_count(ring_buffer_t *buf);
int32_t ring_show_symbol(uint16_t syringmbolNumber, ring_buffer_t *buf);
void ring_clear(ring_buffer_t *buf);
void ring_init(ring_buffer_t *buf, uint16_t size);

#endif

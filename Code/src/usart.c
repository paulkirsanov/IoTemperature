#include "usart.h"

void usart_send(char data)
{
	while(!(USART2->SR & USART_SR_TXE));
	USART2->DR = data;
}

void usart_send_string(char* str)
{
	uint8_t i = 0;
	while(str[i])
		usart_send(str[i++]);
}

void usart2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	GPIOA->CRL &= ~GPIO_CRL_CNF2;
	GPIOA->CRL |= GPIO_CRL_CNF2_1;
	GPIOA->CRL |= GPIO_CRL_MODE2_0;
	
	GPIOA->CRL &= ~GPIO_CRL_CNF3;
	GPIOA->CRL |= GPIO_CRL_CNF3_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE3_0;
	
	USART2->BRR = 0x009C; //baudrate 115200
	
	USART2->CR1 |= USART_CR1_UE;
	USART2->CR1 |= USART_CR1_TE;
	USART2->CR1 |= USART_CR1_RE;
	USART2->CR1 |= USART_CR1_RXNEIE;
	
	USART2->CR1 &= ~USART_CR1_M;
	USART2->CR2 &= ~USART_CR2_STOP;
	
	NVIC_EnableIRQ(USART2_IRQn);
}

void ring_init(ring_buffer_t *buf, uint16_t size)
{
	buf->size = size;
	buf->buffer = (uint8_t*)malloc(size);
	ring_clear(buf);
}

void ring_clear(ring_buffer_t *buf)
{
	buf->idxIn = 0;
	buf->idxOut = 0;
}

void ring_put(uint8_t symbol, ring_buffer_t *buf)
{
	buf->buffer[buf->idxIn++] = symbol;
	if(buf->idxIn >= buf->size) buf->idxIn = 0;
}

uint8_t ring_pop(ring_buffer_t *buf)
{
	uint8_t retval = buf->buffer[buf->idxOut++];
	if(buf->idxOut >= buf->size) buf->idxOut = 0;
	return retval;
}

uint16_t ring_get_count(ring_buffer_t *buf)
{
    uint16_t retval = 0;
    if (buf->idxIn < buf->idxOut) retval = buf->size + buf->idxIn - buf->idxOut;
    else retval = buf->idxIn - buf->idxOut;
    return retval;
}

int32_t ring_show_symbol(uint16_t symbolNumber, ring_buffer_t *buf)
{
    uint32_t pointer = buf->idxOut + symbolNumber;
    int32_t  retval = -1;
    if (symbolNumber < ring_get_count(buf))
    {
        if (pointer > buf->size) pointer -= buf->size;
        retval = buf->buffer[ pointer ] ;
    }
    return retval;
}

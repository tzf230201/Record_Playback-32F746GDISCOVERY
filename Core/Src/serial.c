/*
 * serial.c
 *
 *  Created on: Mar 23, 2021
 *      Author: Teuku Zikri Fatahillah [EE18]
 *  			Bianca Surya Nobelia [ME18]
 */

#include "serial.h"


void serialInit(Serial *serial,USART_TypeDef *USARTx,uint8_t *buffer,uint16_t size)
{
	serial->port_name = USARTx;
	serial->buffer = buffer;
	serial->buffer_size = size;
	serial->current_index= 0;
}

void serialFlush(Serial *serial)
{
	while(!(serial->port_name->ISR & USART_ISR_TXE));
		// serial->port_name->SR &= ~USART_SR_TXE;
}

void serialWrite(Serial *serial,uint8_t value)
{
	serial->port_name->TDR = value;
	serialFlush(serial);
}

void serialPrint(Serial *serial,const char *fmt, ...)
{
    char buffer[64];
    va_list args;
    va_start(args, fmt);
    int length = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    for(int i=0;i<length;i++)
	{
    	serialWrite(serial,(uint8_t)buffer[i]);
	}
}

void serialPrintln(Serial *serial,const char *fmt, ...)
{
    char buffer[64];
    va_list args;
    va_start(args, fmt);
    int length = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    for(int i = 0;i < length;i++)
	{
    	serialWrite(serial,(uint8_t)buffer[i]);
	}
    serialWrite(serial,'\n');
}

uint8_t serialRead(Serial *serial)
{
	uint8_t output = serial->buffer[0];

	for(int i = 0; i < serial->buffer_size-1; i++)
	{
		serial->buffer[i] = serial->buffer[i+1];
	}
	serial->buffer[serial->buffer_size-1]=0;

	if(serial->current_index>0)
	{
		serial->current_index--;
	}

	return(output);
}

void serialForcePut(Serial *serial,uint8_t value)
{
	serial->buffer[serial->current_index] = value;
	serial->current_index++;

	if(serial->current_index>serial->buffer_size)
	{
		serialRead(serial);
	}
}

void serialClear(Serial *serial)
{
	serial->port_name->ISR &= ~USART_ISR_RXNE;
	int temp = serial->port_name->RDR;
	serial->current_index=0;
}

uint8_t usart1_buffer[64];
Serial vcp;

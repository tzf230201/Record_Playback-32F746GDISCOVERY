/*
 * serial.h
 *
 *  Created on: Mar 23, 2021
 *      Author: Teuku Zikri Fatahillah [EE18]
 *  			Bianca Surya Nobelia [ME18]
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include <stm32f7xx.h>
#include "stdarg.h"
#include "stdio.h"

typedef struct
{
	USART_TypeDef *port_name;
	uint8_t *buffer;
	uint16_t buffer_size;
	uint16_t current_index;
}Serial;

void serialInit(Serial *serial,USART_TypeDef *USARTx,uint8_t *buffer,uint16_t size);
void serialFlush(Serial *serial);
void serialWrite(Serial *serial,uint8_t value);
void serialPrint(Serial *serial,const char *fmt, ...);
void serialPrintln(Serial *serial,const char *fmt, ...);
uint8_t serialRead(Serial *serial);
void serialForcePut(Serial *serial,uint8_t value);
void serialClear(Serial *serial);

extern uint8_t usart1_buffer[64];
extern uint8_t usart2_buffer[64];
extern uint8_t usart3_buffer[64];
extern uint8_t usart6_buffer[64];

extern Serial vcp;

#endif /* INC_SERIAL_H_ */

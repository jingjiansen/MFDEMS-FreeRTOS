#ifndef __USART_COM_H
#define __USART_COM_H

#include "stm32f10x.h"
#include <stdio.h>

void USARTX_SendByte(USART_TypeDef *pusartx, uint8_t ch);
void USARTX_SendArray(USART_TypeDef *pusartx, uint8_t *array, uint32_t num);
void USARTX_SendString(USART_TypeDef *pusartx, char *str);
#endif /* __USART_COM_H  */

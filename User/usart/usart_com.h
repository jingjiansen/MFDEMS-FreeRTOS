#ifndef __BSP_USART_COM_H
#define __BSP_USART_COM_H

#include "stm32f10x.h"

void USART_SendByte(USART_TypeDef *pusartx,uint8_t ch);
void USART_SendArray(USART_TypeDef *pusartx,uint8_t *array,uint32_t num);
void USART_SendString(USART_TypeDef *pusartx,uint8_t *str);

#endif  /* __BSP_USART_COM_H */

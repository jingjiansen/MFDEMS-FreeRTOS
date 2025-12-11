#ifndef __BSP_SYSTICK_H
#define __BSP_SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
void SysTick_CountPlus(void);
uint64_t SysTick_GetCount(void);
void SysTick_DelayMs(uint64_t time);
#endif /* __BSP_SYSTICK_H */

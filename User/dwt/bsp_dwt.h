#ifndef __BSP_DWT_H
#define __BSP_DWT_H

#include "stm32f10x.h"

/* DWT时间戳相关寄存器定义 */
#define DEMCR           *(uint32_t *)(0xE000EDFC)
#define DWT_CTRL        *(uint32_t *)(0xE0001000)
#define DWT_CYCCNT      *(uint32_t *)(0xE0001004)
#define DEMCR_TRCENA            (1<<24)
#define DWT_CTRL_CYCCNTENA      (1<<0)

void DWT_Init(void);
uint32_t DWT_GetTick(void);
uint32_t DWT_TickToMicrosecond(uint32_t tick,uint32_t frequency);

void DWT_DelayUs(uint32_t time);
void DWT_DelayMs(uint32_t time);
void DWT_DelayS(uint32_t time);

#endif /* __BSP_DWT_H  */

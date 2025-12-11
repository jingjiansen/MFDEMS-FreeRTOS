#ifndef __BSP_DWT_H
#define __BSP_DWT_H

#include "stm32f10x.h"


/*

使能CYCCNT计数的操作步骤：
    1：先使能DWT外设准备，由内核调试寄存器DEMCR的位24控制，写1使能
    2：使能CYCCNT寄存器之前，先清零
    3：使能CYCCNT计数器，由DWT_CTRL的位0控制，写1使能
*/


#define DEMCR       *(uint32_t *)(0xE000EDFC)
#define DWT_CTRL    *(uint32_t *)(0xE0001000)
#define DWT_CYCCNT  *(uint32_t *)(0xE0001004)

#define DEMCR_TRCENA        (1<<24) /* TRCENA */
#define DWT_CTRL_CYCCNTENA  (1<<0)  /* CYCCNTENA */


void DWT_Init(void);
uint32_t DWT_GetTick(void);
uint32_t DWT_TickToUs(uint32_t tick, uint32_t frequency);
void DWT_Delay_Us(__IO uint32_t time);
void DWT_Delay_Ms(__IO uint32_t time);
void DWT_Delay_S(__IO uint32_t time);

#endif  /* __BSP_DWT_H */

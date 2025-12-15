#include "dwt/bsp_dwt.h"   


/* 初始化DWT外设 */
void DWT_Init(void)
{
    /* 使能DWT外设 */
    DEMCR |= (uint32_t)DEMCR_TRCENA;
    
    /* DWT CYCCNT寄存器计数清0，使能CYCCNT寄存器之前，先清0 */
    DWT_CYCCNT = (uint32_t)0U;
    
    /* 使能Cortex-M DWT CYCCNT寄存器 */
    DWT_CTRL  |=(uint32_t)DWT_CTRL_CYCCNTENA;
}


/* 获取当前时间戳 */
uint32_t DWT_GetTick(void)
{ 
    return ((uint32_t)DWT_CYCCNT);
}


/**
  * @brief  节拍数转化时间间隔(微秒单位)
  * @param  tick :需要转换的节拍数
  * @param  frequency :内核时钟频率
  * @retval 当前时间戳(微秒单位)
  */
uint32_t DWT_TickToMicrosecond(uint32_t tick,uint32_t frequency)
{ 
    return (uint32_t)(1000000.0/frequency*tick);
}


/* DWT精确微秒级延时 */
void DWT_DelayUs(uint32_t time)
{
    /* 将微秒转化为对应的时钟计数值*/
    uint32_t tick_duration= time * (SystemCoreClock / 1000000) ;
    uint32_t tick_start = DWT_GetTick(); /* 刚进入时的计数器值 */
    while(DWT_GetTick() - tick_start < tick_duration); /* 延时等待 */
}


/* DWT精确毫秒级延时 */
void DWT_DelayMs(uint32_t time)
{
    for(uint32_t i = 0; i < time; i++)
    {
        DWT_DelayUs(1000);
    }
}


/* DWT精确秒级延时 */
void DWT_DelayS(uint32_t time)
{
    for(uint32_t i = 0; i < time; i++)
    {
        DWT_DelayMs(1000);
    }
}

/*********************************************END OF FILE**********************/


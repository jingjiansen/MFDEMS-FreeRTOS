#include "systick/bsp_systick.h"

static uint64_t systick_count = 0;


/* 初始化SysTick，配置为1ms中断 */
void SysTick_Init(void)
{
    /* 系统时钟SystemCoreClock 为72MHz，即1s电平翻转72000000次，因此
       为配置1ms中断，需要设置的tick数为 SystemCoreClock/1000 = 72000 */
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1); /* 初始化失败后一直死循环在这，也可方便debug排查 */
    }
    // SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk; /* 关闭定时器(中断) */
}


/* 增加SysTick循环计数 */
void SysTick_CountPlus(void)
{
    systick_count++;
}


/* 查询当前SysTick循环计数的计数值 */
uint64_t SysTick_GetCount(void)
{
    return systick_count;
}


/* 阻塞延时Ms */
void SysTick_DelayMs(uint64_t time)
{
    uint64_t tick_start = SysTick_GetCount();/* 刚进入时的计数器值 */
    while(SysTick_GetCount()-tick_start < time);
}

/*****************************END OF FILE***************************************/

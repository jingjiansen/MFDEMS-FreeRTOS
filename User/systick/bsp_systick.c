/**
  ******************************************************************************
  * @file       bsp_systick.c
  * @author     embedfire
  * @version     V1.0
  * @date        2024
  * @brief      系统定时器函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台  ：野火 STM32F103C8T6-STM32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */

#include "systick/bsp_systick.h"

static uint64_t systick_count = 0;

// 当counter 减1的时间 等于 1/systick_clk
// 当counter 从 reload 的值减小到0的时候，为一个循环，如果开启了中断则执行中断服务程序，
// 同时 CTRL 的 countflag 位会置1
// 这一个循环的时间为 reload * (1/systick_clk)

/**
  * @brief  初始化 SYSTICK （默认使用ms中断）
  * @param  无
  * @note   无
  * @retval 无
  */
void SysTick_Init(void)
{
    /* SystemCoreClock / 1000           1ms中断一次
     * SystemCoreClock / 100000         10us中断一次
     * SystemCoreClock / 1000000        1us中断一次
     */
    if(SysTick_Config(SystemCoreClock/1000))    //1ms中断一次
    {
        while(1);   //初始化失败后一直死循环在这，也可方便debug排查
    }
    
//    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;    //关闭定时器(中断)
}

/**
  * @brief  SysTick 循环计数函数
  * @param  无
  * @note   在 SysTick 中断函数 SysTick_Handler()内调用
  * @retval 无
  */
void SysTick_CountPlus(void)
{
    systick_count++;
}

/**
  * @brief  查询当前 SysTick 循环计数的计数值
  * @param  无
  * @note   无
  * @retval 当前 SysTick 循环计数的计数值
  */
uint64_t SysTick_GetCount(void)
{
    return systick_count;
}

/**
  * @brief  systick 查询寄存器延时函数接口 单位：Ms
  * @param  time：传入计数值
  * @note   使用中断 ,调用此函数时应该在 SysTick 中断函数 SysTick_Handler()的函数体中调用 SysTick_CountPlus()函数
  * @retval 无
  */
void SysTick_DelayMs(uint64_t time)
{
    uint64_t tick_start = SysTick_GetCount();/* 刚进入时的计数器值 */
    while(SysTick_GetCount()-tick_start < time);
}

/*****************************END OF FILE***************************************/

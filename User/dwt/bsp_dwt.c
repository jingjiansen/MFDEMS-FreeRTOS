/**
  ******************************************************************************
  * @file       bsp_dwt.c
  * @author     embedfire
  * @version     V1.0
  * @date        2025
  * @brief      使用DWT定时器（内核定时器）实现精准延时函数
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

#include "dwt/bsp_dwt.h"

/**
  * @brief  初始化 DWT 计数器
  * @param  无
  * @note   使用延时函数之前必须调用本函数进行初始化以开启DWT
  * @retval 无
  */
void DWT_Init(void)
{
    /* 先使能DWT外设准备（位24）*/
    DEMCR |= (uint32_t)DEMCR_TRCENA;
    /* DWT CYCCNT寄存器计数清零 */
    DWT_CYCCNT = (uint32_t)0U;
    /* 使能Cortex-M DWT CYCCNT寄存器（位0） */
    DWT_CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA;
}


/**
  * @brief  获取当前时间戳
  * @param  无
  * @note   无
  * @retval 当前时间戳
  */
uint32_t DWT_GetTick(void)
{
    return ((uint32_t)DWT_CYCCNT);
}

/* 将时钟Tick转换为Us（微秒） */
uint32_t DWT_TickToUs(uint32_t tick, uint32_t frequency)
{
    return (uint32_t)(1000000.0/frequency * tick); /* 使用.0转换为浮点数计算，防止小于零时直接被截断 */
}

/* 微秒级延时函数 */
void DWT_Delay_Us(__IO uint32_t time)
{
    uint32_t tick_duration = time * (SystemCoreClock / 1000000); /* 需要的tick数 */
    
    uint32_t tick_start = DWT_GetTick();
    
    while(DWT_GetTick() - tick_start < tick_duration);
}

/* 毫秒级延时函数 */
void DWT_Delay_Ms(__IO uint32_t time)
{
    for(uint32_t i = 0; i<time; i++)
    {
        DWT_Delay_Us(1000);
    }
}

/* 秒级延时函数 */
void DWT_Delay_S(__IO uint32_t time)
{
    for(uint32_t i = 0; i<time; i++)
    {
        DWT_Delay_Ms(1000);
    }
}



/*****************************END OF FILE***************************************/

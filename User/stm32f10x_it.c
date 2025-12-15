/**
  ******************************************************************************
  * @file    BKP/Tamper/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.6.0
  * @date    20-September-2021
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2011 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "main.h"
#include "debug/bsp_debug.h"
#include "systick/bsp_systick.h"
#include "key/bsp_gpio_key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup BKP_Tamper
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

///**
//  * @brief  This function handles SVCall exception.
//  * @param  None
//  * @retval None
//  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

///**
//  * @brief  This function handles PendSV_Handler exception.
//  * @param  None
//  * @retval None
//  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//    SysTick_CountPlus();
//    if(dht11_rd_task.timer > 0x00)
//    {
//        dht11_rd_task.timer--;
//        if(dht11_rd_task.timer == 0)
//        {
//            dht11_rd_task.flag = 1;
//        }
//    }
//}

/**
  * @brief  该中断负责FreeRTOS的系统时钟，默认1ms中断一次
  * @param  None
  * @retval None
  */
extern void xPortSysTickHandler(void);
void SysTick_Handler(void)
{
    #if (INCLUDE_xTaskGetSchedulerState == 1)
        if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        {
    #endif
            /* 调用FreeRTOS的系统时钟处理函数 */
            xPortSysTickHandler();
    #if (INCLUDE_xTaskGetSchedulerState == 1)
        }
    #endif
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles KEY1 interrupt request.
  * @param  None
  * @retval None
  */
//void KEY1_EXTI_IRQHANDLER(void)
//{
//    if(EXTI_GetFlagStatus(KEY1_EXTI_LINE) == SET)
//    {
//        left_shift_flag = 1; /* 标记左移 */
//        menu_show_flag = 1;  /* 标记显示菜单 */
//        EXTI_ClearITPendingBit(KEY1_EXTI_LINE); /* 清除中断标志位 */
//    }
//}

void KEY1_EXTI_IRQHANDLER(void)
{
    /* 临界段保护：保存并设置中断掩码后在退出时恢复，作用是防止在ISR内部被更高优先级中断
       打断，或保护对某些共享寄存器/操作的完整性。注意：如果只使用FromISR API并确保NVIC
       优先级已配置正确，这个临界段通常不是必须的，过度使用会延长中断响应时间 */
    uint32_t ulReturn;
    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    /* 检查是否是KEY1对应的EXTI线发生了中断，作用是在多源共享一个IRQ时判定具体来源 */
    if(EXTI_GetITStatus(KEY1_EXTI_LINE) == SET)
    {
        left_shift_flag = 1; /* 标记左移 */
        menu_show_flag = 1;  /* 标记显示菜单 */
        EXTI_ClearITPendingBit(KEY1_EXTI_LINE);
    }
    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

void KEY23_EXTI_IRQHANDLER(void)
{
    uint32_t ulReturn;
    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    if(EXTI_GetITStatus(KEY2_EXTI_LINE) == SET)
    {
        right_shift_flag = 1; /* 标记右移 */
        menu_show_flag = 1;   /* 标记显示菜单 */
        EXTI_ClearITPendingBit(KEY2_EXTI_LINE);
    }

    if(EXTI_GetITStatus(KEY3_EXTI_LINE) == SET)
    {
        enter_flag = 1;      /* 标记确认 */
        menu_show_flag = 1;  /* 标记显示菜单 */
        EXTI_ClearITPendingBit(KEY3_EXTI_LINE);
    }

    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 


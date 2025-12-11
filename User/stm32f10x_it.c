/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
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

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* 开发板硬件 */
#include "led/bsp_gpio_led.h"
#include "key/bsp_gpio_key.h"
#include "usart/bsp_usart.h"



/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
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

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @note   该中断已在FreeRTOS中的port.c中实现，用户无需再实现
  * @retval None
  */
// void SVC_Handler(void)
// {
// }

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @note   该中断已在FreeRTOS中的port.c中实现，用户无需再实现
  * @retval None
  */
// void PendSV_Handler(void)
// {
// }

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

extern SemaphoreHandle_t BinarySem_Handle;

extern QueueHandle_t Test_Queue;
static uint32_t send_data1 = 1;
static uint32_t send_data2 = 2;

void KEY1_EXTI_IRQHANDLER(void)
{
    /* 视觉反馈：实际应用应当取消以保持ISR短小 */
    LED_TOGGLE(R_LED_GPIO_PORT, R_LED_GPIO_PIN);

    /* 用于FromISR API返回是否需要做上下文切换 */
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    /* 临界段保护：保存并设置中断掩码后在退出时恢复，作用是防止在ISR内部被更高优先级中断
       打断，或保护对某些共享寄存器/操作的完整性。注意：如果只使用FromISR API并确保NVIC
       优先级已配置正确，这个临界段通常不是必须的，过度使用会延长中断响应时间 */
    uint32_t ulReturn;
    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    /* 检查是否是KEY1对应的EXTI线发生了中断，作用是在多源共享一个IRQ时判定具体来源 */
    if(EXTI_GetITStatus(KEY1_EXTI_LINE) == SET)
    {
        /* 将事件（按键编号或数据）放入消息队列，供任务在线程上下文中处理 */
        xQueueSendFromISR(Test_Queue, &send_data1, &pxHigherPriorityTaskWoken);
        /* 如果前者唤醒了高优先级任务，则请求PendSV做上下文切换，以便更高优先级任务立即运行 */
        portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );

        /* 清除EXTI的中断挂起位，防止中断再次被立即触发，必须做的清理步骤 */
        EXTI_ClearITPendingBit(KEY1_EXTI_LINE);
    }
    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

void KEY2_EXTI_IRQHANDLER(void)
{
    LED_TOGGLE(G_LED_GPIO_PORT, G_LED_GPIO_PIN);

    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulReturn;

    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    if(EXTI_GetITStatus(KEY2_EXTI_LINE) == SET)
    {
        xQueueSendFromISR(Test_Queue, &send_data2, &pxHigherPriorityTaskWoken);

        portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );

        EXTI_ClearITPendingBit(KEY2_EXTI_LINE);
    }

    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

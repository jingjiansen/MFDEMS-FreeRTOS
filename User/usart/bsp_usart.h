#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define USART_USART_NUM 1

#if(USART_USART_NUM == 1)
    /* 发送GPIO引脚配置 */
    #define USART_TX_GPIO_PORT          GPIOA                           /* GPIO端口 */
    #define USART_TX_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
    #define USART_TX_GPIO_PIN           GPIO_Pin_9                      /* 对应PIN脚 */

    /* 接收GPIO引脚配置 */
    #define USART_RX_GPIO_PORT          GPIOA                           /* GPIO端口 */
    #define USART_RX_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
    #define USART_RX_GPIO_PIN           GPIO_Pin_10                     /* 对应PIN脚 */

    /* 串口模式配置 */
    #define USART_USARTX                USART1                          /* 串口号 */
    #define USART_CLK_PORT        RCC_APB2Periph_USART1           /* USART串口时钟 */
    #define USART_APBXCLKCMD            RCC_APB2PeriphClockCmd          /* APBX时钟使能命令 */    
    #define USART_BAUDRATE        115200                          /* 波特率 */
    #define USART_WORDLENGTH      USART_WordLength_8b             /* 字长 */
    #define USART_STOPBITS        USART_StopBits_1                /* 停止位 */
    #define USART_PARITY          USART_Parity_No                 /* 校验位 */
    #define USART_MODE            USART_Mode_Tx|USART_Mode_Rx     /* 模式 */
    #define USART_FLOWCTRL        USART_HardwareFlowControl_None  /* 流控 */
    
    /* 中断配置：中断号和中断处理函数*/
    #define USART_IRQ                   USART1_IRQn
    #define USART_IRQHANDLER            USART1_IRQHandler

#endif

#define USART_BUFFER_SIZE 1024 /* 接收缓冲区大小 */

typedef struct 
{
    uint8_t buffer[USART_BUFFER_SIZE];  /* 接收数据缓存 */
    uint32_t len;                       /* 接收数据长度 */
    uint32_t read_flag;                 /* 接收数据标志位，0可接收数据；1不可接收数据 */
}USART_DataTypeDef;

/* 供main.c中的任务读取 */
extern USART_DataTypeDef usart_receive;

/* 来自于main.c，在ISR中用于给任务发送信号 */
extern SemaphoreHandle_t BinarySem_Handle;

void USART_NVICCONFIG(void);
void USART_GPIOCONFIG(void);
void USART_MODECONFIG(void);
void USART_INIT(void);

#endif  /* __BSP_USART_H */

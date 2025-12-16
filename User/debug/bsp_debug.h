#ifndef __BSP_DEBUG_H
#define __BSP_DEBUG_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

#define DEBUG_UART


/* 系统阶段定义 */
typedef enum {
    SYS_PHASE_BOOT = 0,      /* Bootloader阶段 */
    SYS_PHASE_EARLY,         /* RTOS启动前阶段 */
    SYS_PHASE_RTOS_RUNNING   /* RTOS运行阶段 */
} SystemPhase_t;

/* 串口工作模式 */
typedef enum {
    UART_MODE_POLLING = 0,   /* 轮询模式 */
    UART_MODE_INTERRUPT,     /* 中断模式 */
    UART_MODE_DMA            /* DMA模式 */
} UartMode_t;

/* 串口句柄 */
typedef struct UART_Handle {
    USART_TypeDef *Instance;     /* USART实例 */
    
    /* DMA相关 */
    DMA_Channel_TypeDef *TxDMAChannel;      /* 发送DMA通道 */
    DMA_Channel_TypeDef *RxDMAChannel;      /* 接收DMA通道 */
    uint8_t *TxDMABuffer;                   /* 发送DMA缓冲区 */
    uint16_t TxDMABufferSize;               /* 发送DMA缓冲区大小 */
    volatile uint8_t TxDMAInProgress;       /* DMA发送进行中标志 */
    
    /* RTOS同步对象 */
    SemaphoreHandle_t TxMutex;              /* 发送互斥锁 */
    SemaphoreHandle_t TxCompleteSem;        /* 发送完成信号量 */
    QueueHandle_t RxQueue;                  /* 接收队列 */
    
    /* 环形缓冲区 */
    uint8_t *RxBuffer;                      /* 接收缓冲区 */
    uint16_t RxBufferSize;                  /* 接收缓冲区大小 */
    volatile uint16_t RxHead;               /* 写入指针 */
    volatile uint16_t RxTail;               /* 读取指针 */
    
    /* 配置 */
    uint32_t BaudRate;                      /* 波特率 */    
    UartMode_t CurrentMode;                 /* 当前工作模式 */
    SystemPhase_t SystemPhase;              /* 当前系统阶段 */
    
    /* 回调函数 */
    void (*RxCallback)(struct UART_Handle *huart, uint8_t data);   /* 接收回调函数 */
    void (*IdleCallback)(struct UART_Handle *huart);               /* 空闲回调函数 */
} UART_Handle_t;

/* 调试串口定义 */
#ifdef DEBUG_UART
    #define TXDMABUFFER_SIZE    (256)           /* 发送DMA缓冲区大小 */
    #define RXDMABUFFER_SIZE    (1024)          /* 接收DMA缓冲区大小 */

    #define QUEUESIZE           (64)            /* 接收队列大小 */

    /* 发送引脚定义 */
    #define TX_GPIO_PORT       GPIOA
    #define TX_GPIO_PIN        GPIO_Pin_9
    #define TX_GPIO_MODE       GPIO_Mode_AF_PP
    #define TX_GPIO_SPEED      GPIO_Speed_50MHz
    #define TX_GPIO_CLK        RCC_APB2Periph_GPIOA

     /* 接收引脚定义 */
    #define RX_GPIO_PORT       GPIOA
    #define RX_GPIO_PIN        GPIO_Pin_10
    #define RX_GPIO_MODE       GPIO_Mode_IPU
    #define RX_GPIO_SPEED      GPIO_Speed_50MHz
    #define RX_GPIO_CLK        RCC_APB2Periph_GPIOA

    #define DEBUG_USART                 USART1
    #define DEBUG_CLK                   RCC_APB2Periph_USART1
    #define DEBUG_USART_WORDLENGTH      USART_WordLength_8b
    #define DEBUG_USART_STOPBITS        USART_StopBits_1
    #define DEBUG_USART_PARITY          USART_Parity_No
    #define DEBUG_USART_HWCONTROL       USART_HardwareFlowControl_None
    #define DEBUG_USART_MODE            (USART_Mode_Tx | USART_Mode_Rx)

    #define DEBUG_BAUDRATE              115200
    #define DEBUG_TX_DMA_CH             DMA1_Channel4
    #define DEBUG_RX_DMA_CH             DMA1_Channel5

    extern UART_Handle_t DebugUart;
#endif

/* 初始化函数 */
void UART_Driver_Init(void);            /* 硬件初始化：GPIO、USART */
void UART_SwitchToRTOSMode(void);       /* 模式切换：从轮询模式转换为DMA发送，中断接收（配置RTOS资源：互斥信号量、队列、缓冲区、DMA初始化、NVIC配置） */

/* 阶段1：启动前发送函数（阻塞） */
void UART_Early_SendByte(UART_Handle_t *huart, uint8_t data);
void UART_Early_SendString(UART_Handle_t *huart, const char *str);
void UART_Early_SendBuffer(UART_Handle_t *huart, uint8_t *buffer, uint16_t len);

/* 阶段2：RTOS下发送函数（DMA，非阻塞） */
BaseType_t UART_DMA_Send(UART_Handle_t *huart, uint8_t *data, uint16_t len, TickType_t timeout);
BaseType_t UART_DMA_SendString(UART_Handle_t *huart, const char *str, TickType_t timeout);

/* 接收函数 */
BaseType_t UART_ReceiveByte(UART_Handle_t *huart, uint8_t *data, TickType_t timeout);
uint16_t UART_ReceiveBuffer(UART_Handle_t *huart, uint8_t *buffer, uint16_t len, TickType_t timeout);

/* 中断处理函数 */
void USART1_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);

/* 重定向printf（自动适应阶段） */
int uart_printf(const char *format, ...);

#endif /* __BSP_DEBUG_H  */

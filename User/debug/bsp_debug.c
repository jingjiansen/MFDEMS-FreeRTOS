#include "debug/bsp_debug.h"
#include "usart/usart_com.h"

#include <stdarg.h>
#include <string.h>

/* 调试串口全局句柄 */
#ifdef DEBUG_UART
UART_Handle_t DebugUart = {
    .Instance = DEBUG_USART,
    .TxDMAChannel = DEBUG_TX_DMA_CH,
    .RxDMAChannel = DEBUG_RX_DMA_CH,
    .TxDMABuffer = NULL,
    .TxDMABufferSize = TXDMABUFFER_SIZE,
    .TxDMAInProgress = 0,
    .TxMutex = NULL,
    .TxCompleteSem = NULL,
    .RxQueue = NULL,
    .RxBuffer = NULL,
    .RxBufferSize = RXDMABUFFER_SIZE,
    .RxHead = 0,
    .RxTail = 0,
    .BaudRate = DEBUG_BAUDRATE,
    .CurrentMode = UART_MODE_POLLING,
    .SystemPhase = SYS_PHASE_EARLY,
    .RxCallback = NULL,
    .IdleCallback = NULL
};
#endif

/* 私有函数声明 */
static void UART_InitDMA(UART_Handle_t *huart);
static void UART_InitNVIC(UART_Handle_t *huart);
static void UART_EnableInterrupts(UART_Handle_t *huart);
static void UART_DisableInterrupts(UART_Handle_t *huart);

/*******************************************************************************
 * 阶段1：RTOS启动前的初始化（极简初始化）
 ******************************************************************************/

void UART_Driver_Init(void) 
{
#ifdef DEBUG_UART
    /* 早期初始化：仅配置GPIO和USART，使用轮询模式 */
    RCC_APB2PeriphClockCmd(TX_GPIO_CLK | RX_GPIO_CLK | DEBUG_CLK, ENABLE);
    
    /* 配置TX为推挽复用输出 */
    GPIO_InitTypeDef GPIO_InitStruct = 
    {
        .GPIO_Pin = TX_GPIO_PIN,
        .GPIO_Mode = TX_GPIO_MODE,
        .GPIO_Speed = TX_GPIO_SPEED
    };
    GPIO_Init(TX_GPIO_PORT, &GPIO_InitStruct);
    
    /* 配置RX为上拉输入 */
    GPIO_InitStruct.GPIO_Pin = RX_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = RX_GPIO_MODE;
    GPIO_Init(RX_GPIO_PORT, &GPIO_InitStruct);
    
    /* 配置USART */
    USART_InitTypeDef USART_InitStruct = 
    {
        .USART_BaudRate = DebugUart.BaudRate,
        .USART_WordLength = DEBUG_USART_WORDLENGTH,
        .USART_StopBits = DEBUG_USART_STOPBITS,
        .USART_Parity = DEBUG_USART_PARITY,
        .USART_HardwareFlowControl = DEBUG_USART_HWCONTROL,
        .USART_Mode = DEBUG_USART_MODE
    };
    USART_Init(DebugUart.Instance, &USART_InitStruct);
    
    USART_Cmd(DebugUart.Instance, ENABLE);
    
    DebugUart.CurrentMode = UART_MODE_POLLING;
    DebugUart.SystemPhase = SYS_PHASE_EARLY;    
#endif
}

/*******************************************************************************
 * 阶段1：启动前发送函数（阻塞轮询）
 ******************************************************************************/

void UART_Early_SendByte(UART_Handle_t *huart, uint8_t data) 
{
    if (huart->SystemPhase != SYS_PHASE_RTOS_RUNNING) 
    {
        /* 等待发送寄存器空（TXE为1表示数据已从DR寄存器转移到发送移位寄存器） */
        while (USART_GetFlagStatus(huart->Instance, USART_FLAG_TXE) == RESET);
        
        /* 发送数据 */
        USART_SendData(huart->Instance, data);
        
        /* 等待发送完成（TC为1表示数据已经发出，移位寄存器为空） */
        while (USART_GetFlagStatus(huart->Instance, USART_FLAG_TC) == RESET);
    }
}

void UART_Early_SendString(UART_Handle_t *huart, const char *str) 
{
    while (*str) 
    {
        UART_Early_SendByte(huart, *str);
        str++;
        
        /* 如果是换行，补回车 */
        if (*(str - 1) == '\n' && *str != '\r') 
        {
            UART_Early_SendByte(huart, '\r');
        }
    }
}

void UART_Early_SendBuffer(UART_Handle_t *huart, uint8_t *buffer, uint16_t len) 
{
    for (uint16_t i = 0; i < len; i++) 
    {
        UART_Early_SendByte(huart, buffer[i]);
    }
}


/*******************************************************************************
 * 阶段2：切换到RTOS模式（中断接收 + DMA发送）
 ******************************************************************************/

void UART_SwitchToRTOSMode(void) 
{
#ifdef DEBUG_UART
    /* 1. 禁止USART和中断 */
    UART_DisableInterrupts(&DebugUart);
    USART_Cmd(DebugUart.Instance, DISABLE);
    
    /* 2. 创建RTOS资源 */
    DebugUart.TxMutex = xSemaphoreCreateMutex();
    DebugUart.TxCompleteSem = xSemaphoreCreateBinary();
    DebugUart.RxQueue = xQueueCreate(QUEUESIZE, sizeof(uint8_t));
    
    /* 3. 分配缓冲区 */
    DebugUart.RxBuffer = pvPortMalloc(DebugUart.RxBufferSize);
    DebugUart.TxDMABuffer = pvPortMalloc(DebugUart.TxDMABufferSize);
    memset(DebugUart.RxBuffer, 0, DebugUart.RxBufferSize);
    memset(DebugUart.TxDMABuffer, 0, DebugUart.TxDMABufferSize);
    
    /* 4. 重新配置为中断+DMA模式 */
    UART_InitDMA(&DebugUart);
    UART_InitNVIC(&DebugUart);
    
    /* 5. 重新使能USART */
    USART_Cmd(DebugUart.Instance, ENABLE);
    UART_EnableInterrupts(&DebugUart);
    
    /* 6. 启动DMA接收 */
    USART_DMACmd(DebugUart.Instance, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DebugUart.RxDMAChannel, ENABLE);
    
    /* 7. 更新状态 */
    DebugUart.CurrentMode = UART_MODE_DMA;
    DebugUart.SystemPhase = SYS_PHASE_RTOS_RUNNING;
    DebugUart.TxDMAInProgress = 0;    
#endif
}


/*******************************************************************************
 * 阶段2：RTOS下发送函数（DMA，非阻塞）
 ******************************************************************************/

BaseType_t UART_DMA_Send(UART_Handle_t *huart, uint8_t *data, uint16_t len, TickType_t timeout) 
{
    BaseType_t result = pdFAIL;
    
    /* 仅RTOS模式下使用 */
    if (huart->SystemPhase != SYS_PHASE_RTOS_RUNNING) 
    {
        return pdFAIL;
    }
    
    /* 获取发送互斥锁 */
    if (xSemaphoreTake(huart->TxMutex, timeout) == pdPASS) 
    {
        /* 检查DMA是否繁忙 */
        if (huart->TxDMAInProgress == 0) 
        {
            /* 复制数据到DMA缓冲区（防止原始数据被修改） */
            uint16_t copy_len = (len > huart->TxDMABufferSize) ? huart->TxDMABufferSize : len;
            memcpy(huart->TxDMABuffer, data, copy_len);
            
            /* 配置DMA传输 */
            DMA_Cmd(huart->TxDMAChannel, DISABLE);  /* 禁用DMA通道 */
            DMA_SetCurrDataCounter(huart->TxDMAChannel, copy_len); /* 设置DMA传输数据计数器 */
            DMA_Cmd(huart->TxDMAChannel, ENABLE);   /* 使能DMA通道 */
            
            /* 启动USART DMA发送 */
            USART_DMACmd(huart->Instance, USART_DMAReq_Tx, ENABLE);
            
            /* 设置DMA传输中标志 */
            huart->TxDMAInProgress = 1;
            
            /* 等待DMA传输完成 */
            if (xSemaphoreTake(huart->TxCompleteSem, timeout) == pdPASS) 
            {
                result = pdPASS;
                huart->TxDMAInProgress = 0;
            }
            else
            {
                /* 超时或错误：禁用DMA并清除标志 */
                DMA_Cmd(huart->TxDMAChannel, DISABLE);
                USART_DMACmd(huart->Instance, USART_DMAReq_Tx, DISABLE);
                huart->TxDMAInProgress = 0;
            }
        }
        
        /* 释放互斥锁 */
        xSemaphoreGive(huart->TxMutex);
    }
    
    return result;
}

BaseType_t UART_DMA_SendString(UART_Handle_t *huart, const char *str, TickType_t timeout) 
{
    return UART_DMA_Send(huart, (uint8_t *)str, strlen(str), timeout);
}

/*******************************************************************************
 * 接收函数（适用于所有阶段）
 ******************************************************************************/

BaseType_t UART_ReceiveByte(UART_Handle_t *huart, uint8_t *data, TickType_t timeout) 
{
    if (huart->SystemPhase == SYS_PHASE_RTOS_RUNNING) 
    {
        /* RTOS模式：从队列接收 */
        return xQueueReceive(huart->RxQueue, data, timeout);
    } 
    else 
    {
        /* 非RTOS模式：轮询接收 */
        uint32_t start_time = xTaskGetTickCount();
        
        while (1) 
        {
            if (USART_GetFlagStatus(huart->Instance, USART_FLAG_RXNE) != RESET) 
            {
                *data = USART_ReceiveData(huart->Instance);
                return pdPASS;
            }
            
            /* 超时检查 */
            if ((xTaskGetTickCount() - start_time) > timeout) 
            {
                return pdFAIL;
            }
        }
    }
}

uint16_t UART_ReceiveBuffer(UART_Handle_t *huart, uint8_t *buffer, uint16_t len, TickType_t timeout) 
{
    uint16_t received = 0;
    uint8_t data;
    TickType_t start_time = xTaskGetTickCount();
    
    while (received < len) 
    {
        if (UART_ReceiveByte(huart, &data, timeout) == pdPASS) 
        {
            buffer[received++] = data;
            start_time = xTaskGetTickCount(); /* 重置超时计时 */
        } 
        else 
        {
            /* 超时 */
            break;
        }
        
        /* 检查总超时 */
        if ((xTaskGetTickCount() - start_time) > timeout) 
        {
            break;
        }
    }
    
    return received;
}

/*******************************************************************************
 * DMA初始化
 ******************************************************************************/

static void UART_InitDMA(UART_Handle_t *huart) 
{
#ifdef DEBUG_UART
    if (huart->Instance == USART1) 
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  /* 使能DMA时钟，DMA1挂载在AHB总线上 */        
        DMA_InitTypeDef DMA_InitStruct;

        /* TX DMA通道配置（发送方向） */
        DMA_DeInit(DEBUG_TX_DMA_CH);                                            /* 重置DMA1通道14 */
        DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);        /* 外设地址：USART1数据寄存器 */
        DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)huart->TxDMABuffer;       /* 内存基地址（发射缓冲区）*/
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;                         /* 发送方向：内存到外设 */
        DMA_InitStruct.DMA_BufferSize = 0;                                      /* 缓冲区大小：动态设置（发送时设定） */
        DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           /* 外设地址不递增 */
        DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                    /* 内存地址递增 */
        DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;    /* 外设数据大小：字节 */
        DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;            /* 内存数据大小：字节 */
        DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;                              /* 正常模式：非循环 */
        DMA_InitStruct.DMA_Priority = DMA_Priority_Low;                         /* 低优先级 */        DMA_InitStruct.DMA_Priority = DMA_Priority_Low;                         /* */
        DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;        
        DMA_Init(DEBUG_TX_DMA_CH, &DMA_InitStruct);
        DMA_ITConfig(DEBUG_TX_DMA_CH, DMA_IT_TC, ENABLE);                       /* 使能DMA传输完成中断 */
        
        /* RX DMA通道配置（接收方向） */
        DMA_DeInit(DEBUG_RX_DMA_CH);                                            /* 重置DMA1通道5 */
        DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);        /* 外设地址：USART1数据寄存器 */
        DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)huart->RxBuffer;          /* 内存基地址（接收缓冲区）*/
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;                         /* 接收方向：外设到内存 */
        DMA_InitStruct.DMA_BufferSize = huart->RxBufferSize;                    /* 缓冲区大小（接收缓冲区大小）*/
        DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           /* 外设地址不递增 */
        DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                    /* 内存地址递增 */
        DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;    /* 外设数据大小：字节 */
        DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;            /* 内存数据大小：字节 */
        DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                            /* 循环模式接收 */
        DMA_InitStruct.DMA_Priority = DMA_Priority_Low;                         /* 低优先级 */
        DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
        DMA_Init(DEBUG_RX_DMA_CH, &DMA_InitStruct);
        DMA_ITConfig(DEBUG_RX_DMA_CH, DMA_IT_TC, ENABLE);                       /* 使能DMA传输完成中断 */
    }
#endif
}

/*******************************************************************************
 * 中断处理
 ******************************************************************************/

/* USART1中断处理 */
void USART1_IRQHandler(void) 
{
#ifdef DEBUG_UART
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;    
    
    /* 空闲中断：一帧数据接收完成 */
    if (USART_GetITStatus(DebugUart.Instance, USART_IT_IDLE) != RESET) 
    {
        /* 调用空闲回调（如果设置） */
        if (DebugUart.IdleCallback != NULL) 
        {
            DebugUart.IdleCallback(&DebugUart);
        }
        
        USART_ReceiveData(DebugUart.Instance); /* 读DR清除空闲标志 */
        USART_ClearITPendingBit(DebugUart.Instance, USART_IT_IDLE);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
}

/* DMA1通道4中断处理（USART1 TX DMA完成） */
void DMA1_Channel4_IRQHandler(void) 
{
#ifdef DEBUG_UART
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (DMA_GetITStatus(DMA1_IT_TC4) != RESET) 
    {
        /* 禁止USART DMA发送请求 */
        USART_DMACmd(DebugUart.Instance, USART_DMAReq_Tx, DISABLE);
        
        /* 清除DMA标志 */
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        
        /* 发送完成信号量 */
        if (DebugUart.TxCompleteSem != NULL) 
        {
            xSemaphoreGiveFromISR(DebugUart.TxCompleteSem, &xHigherPriorityTaskWoken);
        }

        /* 清除传输中标志 */
        DebugUart.TxDMAInProgress = 0;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
}

/* DMA1通道5中断处理（USART1 RX DMA完成） */
void DMA1_Channel5_IRQHandler(void) 
{
#ifdef DEBUG_UART
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* 传输完成中断 */
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET) 
    {
        /* 计算接收到的数据量 */
        uint16_t received_count = DebugUart.RxBufferSize - DMA_GetCurrDataCounter(DebugUart.RxDMAChannel);
        
        /* 处理接收到的数据 */
        if (DebugUart.RxCallback != NULL) 
        {
            DebugUart.RxCallback(&DebugUart, received_count);
        }
        
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
    
    /* 半传输中断（可选，用于大数据量处理） */
    if (DMA_GetITStatus(DMA1_IT_HT5) != RESET) 
    {
        uint16_t half_received = DebugUart.RxBufferSize / 2;
        if (DebugUart.RxCallback != NULL) 
        {
            DebugUart.RxCallback(&DebugUart, half_received);
        }
        DMA_ClearITPendingBit(DMA1_IT_HT5);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
}

/*******************************************************************************
 * NVIC和中断配置
 ******************************************************************************/

static void UART_InitNVIC(UART_Handle_t *huart) 
{
#ifdef DEBUG_UART
    if (huart->Instance == USART1) 
    {
        NVIC_InitTypeDef NVIC_InitStruct;
        
        /* 配置USART1中断（仅空闲中断） */
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 10;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
        
        /* 配置DMA1通道4中断（TX完成） */
        NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
        NVIC_Init(&NVIC_InitStruct);
        
        /* 新增：配置DMA1通道5中断（RX完成） */
        NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel5_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
        NVIC_Init(&NVIC_InitStruct);
    }
#endif
}

static void UART_EnableInterrupts(UART_Handle_t *huart) 
{
    /* 使能USART接收中断和空闲中断 */
    USART_ITConfig(huart->Instance, USART_IT_IDLE, ENABLE);
    
    /* 使能DMA传输完成中断 */
    if (huart->TxDMAChannel != NULL) 
    {
        DMA_ITConfig(huart->TxDMAChannel, DMA_IT_TC, ENABLE);
    }
    /* 使能DMA接收完成中断 */
    if (huart->RxDMAChannel != NULL) 
    {
        DMA_ITConfig(huart->RxDMAChannel, DMA_IT_TC, ENABLE);
        DMA_ITConfig(huart->RxDMAChannel, DMA_IT_HT, ENABLE);  // 半传输中断
    }
}

static void UART_DisableInterrupts(UART_Handle_t *huart) 
{
    // USART_ITConfig(huart->Instance, USART_IT_RXNE, DISABLE);
    USART_ITConfig(huart->Instance, USART_IT_IDLE, DISABLE);
    
    if (huart->TxDMAChannel != NULL) 
    {
        DMA_ITConfig(huart->TxDMAChannel, DMA_IT_TC, DISABLE);
    }
}

/*******************************************************************************
 * 统一的printf函数（自动适应阶段）
 ******************************************************************************/

int uart_printf(const char *format, ...) 
{
    char buffer[256];
    va_list args;
    int len;
    
    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0) 
    {
#ifdef DEBUG_UART
        if (DebugUart.SystemPhase == SYS_PHASE_RTOS_RUNNING) 
        {
            /* RTOS模式：DMA发送 */
            UART_DMA_Send(&DebugUart, (uint8_t *)buffer, len, portMAX_DELAY);
        } 
        else 
        {
            /* 非RTOS模式：轮询发送 */
            UART_Early_SendBuffer(&DebugUart, (uint8_t *)buffer, len);
        }
#endif
    }
    
    return len;
}

/*****************************END OF FILE***************************************/


/* 硬件相关的头文件 */
#include "usart/bsp_usart.h"
#include "usart/usart_com.h"

/* FreeRTOS头文件 */
#include "task.h"

/* 初始化接收数据结构体 */
USART_DataTypeDef usart_receive = {0};


/* NVIC中断配置 */
void USART_NVICCONFIG(void)
{
    /* 开启 AFIO 相关的时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 

    NVIC_InitTypeDef nvic_initstruct = {0};
    nvic_initstruct.NVIC_IRQChannel                     = USART_IRQ;    /* 配置中断源 */
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  8;           /* 配置抢占优先级 */
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;           /* 配置子优先级 */
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;      /* 使能配置中断通道 */
    NVIC_Init(&nvic_initstruct);
}

/* 配置串口GPIO */
void USART_GPIOCONFIG(void)
{
    GPIO_InitTypeDef gpio_initstruct = {0};

    RCC_APB2PeriphClockCmd(USART_TX_GPIO_CLK_PORT, ENABLE); /* 开启串口发送GPIO时钟 */
    gpio_initstruct.GPIO_Pin    = USART_TX_GPIO_PIN;        /* 配置发送引脚 */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_AF_PP;          /* 配置发送引脚为复用推挽输出 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;         /* 配置发送引脚速率为50MHz */
    GPIO_Init(USART_TX_GPIO_PORT,&gpio_initstruct);         /* 初始化发送引脚 */

    RCC_APB2PeriphClockCmd(USART_RX_GPIO_CLK_PORT, ENABLE); /* 开启串口接收GPIO时钟 */    
    gpio_initstruct.GPIO_Pin    = USART_RX_GPIO_PIN;        /* 配置接收引脚 */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IPU;            /* 配置接收引脚为上拉输入 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;         /* 配置接收引脚速率为50MHz */
    GPIO_Init(USART_RX_GPIO_PORT,&gpio_initstruct);         /* 初始化接收引脚 */
}

/* 配置串口工作模式 */
void USART_MODECONFIG(void)
{
    USART_InitTypeDef usart_initstruct = {0};
   
    USART_APBXCLKCMD(USART_CLK_PORT,ENABLE);    
    usart_initstruct.USART_BaudRate                 = USART_BAUDRATE;     /* 配置波特率 */
    usart_initstruct.USART_HardwareFlowControl      = USART_FLOWCTRL;     /* 配置硬件流控制 */
    usart_initstruct.USART_Mode                     = USART_MODE;         /* 配置工作模式 */
    usart_initstruct.USART_Parity                   = USART_PARITY;       /* 配置校验位 */
    usart_initstruct.USART_StopBits                 = USART_STOPBITS;     /* 配置停止位 */
    usart_initstruct.USART_WordLength               = USART_WORDLENGTH;   /* 配置数据位长度 */
    USART_Init(USART_USARTX,&usart_initstruct);
    
    USART_ITConfig(USART_USARTX, USART_IT_RXNE, ENABLE); /* 使能接收字节中断：每接收到1个字节便产生一次中断 */
    USART_ITConfig(USART_USARTX, USART_IT_IDLE, ENABLE); /* 使能空闲中断：当接收数据持续一段时间后无新数据到达时产生中断 */
}

/* 串口初始化 */
void USART_INIT(void)
{
    USART_NVICCONFIG();                /* 配置串口中断 */
    USART_MODECONFIG();                /* 配置串口模式 */    
    USART_GPIOCONFIG();                /* 配置串口引脚 */    
    USART_Cmd(USART_USARTX, ENABLE);   /* 使能串口 */
}


void USART_IRQHANDLER(void)
{
    uint8_t data_temp = 0;

    /* 字节中断被触发 */
    if(USART_GetITStatus(USART_USARTX, USART_IT_RXNE) == SET)
    {
        /* 从当前串口的DR寄存器中读取数据，读操作会自动清除RXNE寄存器 */
        data_temp = USART_ReceiveData(USART_USARTX); 

        /* 只有在缓冲区未溢出且可接收（即目前没有处理数据）时才接收新数据 */
        if(usart_receive.len < USART_BUFFER_SIZE-1 && usart_receive.read_flag == 0)
        {
            usart_receive.buffer[usart_receive.len++] = data_temp;
        }

        /* 接收缓冲区满则强制结束 */
        if(usart_receive.len == USART_BUFFER_SIZE-1)
        {
            usart_receive.buffer[usart_receive.len] = '\0';
            usart_receive.read_flag = 1;
                
            if(BinarySem_Handle != NULL) /* 释放接收任务的二进制信号量 */
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xSemaphoreGiveFromISR(BinarySem_Handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }

        /* 接收到换行符'\n'则强制结束 */
        if(data_temp == '\n')
        {
            usart_receive.buffer[usart_receive.len] = '\0';
            usart_receive.read_flag = 1;

            if(BinarySem_Handle != NULL) /* 释放接收任务的二进制信号量 */
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xSemaphoreGiveFromISR(BinarySem_Handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }

        // 使用USART_ReceiveData函数接收数据时，会自动清除RXNE标志位，无需手动清除
        // USART_ClearITPendingBit(USART_USARTX, USART_IT_RXNE);
    }

    /* 空闲中断被触发 */
    if(USART_GetITStatus(USART_USARTX, USART_IT_IDLE) == SET)
    {
        /*通过先读取IDLE标志位，再读取DR寄存器，来清除IDLE标志位*/
        USART_ReceiveData(USART_USARTX);
        usart_receive.buffer[usart_receive.len] = '\0';
        usart_receive.read_flag = 1;

        if(BinarySem_Handle != NULL) /* 释放接收任务的二进制信号量 */
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(BinarySem_Handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/*****************************END OF FILE***************************************/

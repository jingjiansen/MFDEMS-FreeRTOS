#include "debug/bsp_debug.h"
#include "usart/usart_com.h"

DEBUG_DataTypeDef debug_receive = {0};


/* USART 中断配置 */
void DEBUG_NVIC_Config(void)
{
    NVIC_InitTypeDef nvic_initstruct = {0};                             /* 定义一个 NVIC 结构体 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);                 /* 开启 AFIO 相关的时钟 */
    nvic_initstruct.NVIC_IRQChannel                     = DEBUG_IRQ;    /* 配置中断源 */
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  1;           /* 配置抢占优先级 */
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;           /* 配置子优先级 */
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;      /* 使能配置中断通道 */
    NVIC_Init(&nvic_initstruct);
}


/* 初始化串口IO */
void DEBUG_USART_PinConfig(void)
{
    GPIO_InitTypeDef gpio_initstruct = {0};

    RCC_APB2PeriphClockCmd(DEBUG_TX_GPIO_CLK_PORT,ENABLE);
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_AF_PP;   /* 输出引脚为推挽复用输出 */
    gpio_initstruct.GPIO_Pin    = DEBUG_TX_GPIO_PIN;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(DEBUG_TX_GPIO_PORT,&gpio_initstruct); 

    RCC_APB2PeriphClockCmd(DEBUG_RX_GPIO_CLK_PORT,ENABLE);
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IPU;     /* 输入引脚为上拉输入 */
    gpio_initstruct.GPIO_Pin    = DEBUG_RX_GPIO_PIN;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(DEBUG_RX_GPIO_PORT,&gpio_initstruct); 
}

/* 配置串口模式 */
void DEBUG_USART_ModeConfig(void)
{
    USART_InitTypeDef usart_initstruct = {0};
    DEBUG_APBXCLKCMD(DEBUG_USARTX_CLK_PORT,ENABLE); /* 开启 DEBUG 相关的GPIO外设/端口时钟 */
    usart_initstruct.USART_BaudRate                 =  DEBUG_BAUDRATE;                  /* 配置波特率 */
    usart_initstruct.USART_HardwareFlowControl      =  USART_HardwareFlowControl_None;  /* 配置硬件流控制 */
    usart_initstruct.USART_Mode                     =  USART_Mode_Tx|USART_Mode_Rx;     /* 配置工作模式 */
    usart_initstruct.USART_Parity                   =  USART_Parity_No;                 /* 配置校验位 */    
    usart_initstruct.USART_StopBits                 =  USART_StopBits_1;                /* 配置停止位 */
    usart_initstruct.USART_WordLength               =  USART_WordLength_8b;             /* 配置帧数据字长 */
    USART_Init(DEBUG_USARTX, &usart_initstruct);
    
    USART_ITConfig(DEBUG_USARTX,USART_IT_RXNE,ENABLE); /* 字节接收中断：接收一个字节数据便会触发一次 */
    USART_ITConfig(DEBUG_USARTX,USART_IT_IDLE,ENABLE); /* 空闲中断：接收完成后总线空闲触发一次 */ 
}


/* 串口初始化 */
void DEBUG_USART_Init(void)
{
    DEBUG_NVIC_Config();                /* 配置串口中断 */
    DEBUG_USART_ModeConfig();           /* 配置串口模式 */
    DEBUG_USART_PinConfig();            /* 配置串口引脚 */    
    USART_Cmd(DEBUG_USARTX,ENABLE);
}


/* 串口中断回调函数 */
void DEBUG_IRQHANDLER(void)
{
    uint8_t data_temp = NULL;

    /* 字节接收中断：接收一个字节数据便会触发一次 */
    if(USART_GetITStatus(DEBUG_USARTX, USART_IT_RXNE) == SET)  
    {
        data_temp = USART_ReceiveData(DEBUG_USARTX); /* 读取数据寄存器的数据，读取后对应的寄存器会被复位 */
        
        /* 未接收满且程序不正在读取缓冲区，才把数据添加进缓冲区 */
        if((debug_receive.len < DEBUG_BUFFER_SIZE-1) && debug_receive.read_flag == 0)   
        {
            debug_receive.buffer[debug_receive.len] = data_temp;
            debug_receive.len++;
        }
        /* 缓冲区满强制结束 */
        if(debug_receive.len == DEBUG_BUFFER_SIZE-1)
        {
            debug_receive.buffer[debug_receive.len] = '\0';
            debug_receive.read_flag = 1;
        }
        USART_ClearITPendingBit(DEBUG_USARTX,USART_IT_RXNE);
    }

    /* 空闲中断：接收完成后总线空闲触发一次 */
    if(USART_GetITStatus(DEBUG_USARTX, USART_IT_IDLE) == SET)  
    {
        USART_ReceiveData(DEBUG_USARTX);
        debug_receive.buffer[debug_receive.len] = '\0';
        debug_receive.read_flag = 1;
    }    
}

/*****************************END OF FILE***************************************/


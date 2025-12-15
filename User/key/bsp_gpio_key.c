#include "key/bsp_gpio_key.h"
#include "systick/bsp_systick.h"
#include "dwt/bsp_dwt.h"

KEY_Info key1_info  = {KEY1_GPIO_PORT,KEY1_GPIO_PIN,KEY_GENERAL_TRIGGER,KEY_INIT,0,0,EVENT_ATTONITY,KEY_NONE_CLICK};
KEY_Info key2_info  = {KEY2_GPIO_PORT,KEY2_GPIO_PIN,KEY_GENERAL_TRIGGER,KEY_INIT,0,0,EVENT_ATTONITY,KEY_NONE_CLICK};
KEY_Info key3_info  = {KEY3_GPIO_PORT,KEY3_GPIO_PIN,KEY_GENERAL_TRIGGER,KEY_INIT,0,0,EVENT_ATTONITY,KEY_NONE_CLICK};



/* 按键GPIO配置 */
void KEY_GPIO_Config(void)
{
    GPIO_InitTypeDef gpio_initstruct = {0};                 /* 按键GPIO结构体 */

    RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK_PORT,ENABLE);      /* 开启KEY1相关的GPIO外设/端口时钟 */    
    GPIO_SetBits(KEY1_GPIO_PORT,KEY1_GPIO_PIN);             /* 按键为低电平触发，这里初始化为高电平 */    
    gpio_initstruct.GPIO_Pin    = KEY1_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(KEY1_GPIO_PORT,&gpio_initstruct);
    
    RCC_APB2PeriphClockCmd(KEY2_GPIO_CLK_PORT,ENABLE);
    GPIO_SetBits(KEY2_GPIO_PORT,KEY2_GPIO_PIN);    
    gpio_initstruct.GPIO_Pin    = KEY2_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(KEY2_GPIO_PORT,&gpio_initstruct);

    RCC_APB2PeriphClockCmd(KEY3_GPIO_CLK_PORT,ENABLE);
    GPIO_SetBits(KEY3_GPIO_PORT,KEY3_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = KEY3_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IPD;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(KEY3_GPIO_PORT,&gpio_initstruct);
}


/* 外部中断线配置（GPIO>EXTI） */
void KEY_Mode_Config(void)
{
    EXTI_InitTypeDef exti_initstruct = {0};                         /* 定义一个 EXTI 初始化结构体 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);             /* 开启 AFIO 相关的时钟 */

    GPIO_EXTILineConfig(KEY1_EXTI_PORTSOURCE, KEY1_EXTI_PINSOURCE); /* 选择中断信号源，将KEY1的引脚映射到EXTI0 */
    exti_initstruct.EXTI_Line       = EXTI_Line0;                   /* 选择中断LINE */
    exti_initstruct.EXTI_Mode       = EXTI_Mode_Interrupt;          /* 选择中断模式*/
    exti_initstruct.EXTI_Trigger    = EXTI_Trigger_Falling;         /* 选择触发方式*/
    exti_initstruct.EXTI_LineCmd    = ENABLE;                       /* 使能中断*/
    EXTI_Init(&exti_initstruct);

    GPIO_EXTILineConfig(KEY2_EXTI_PORTSOURCE,KEY2_EXTI_PINSOURCE);    
    exti_initstruct.EXTI_Line       = EXTI_Line13;
    exti_initstruct.EXTI_Mode       = EXTI_Mode_Interrupt;
    exti_initstruct.EXTI_Trigger    = EXTI_Trigger_Falling;
    exti_initstruct.EXTI_LineCmd    = ENABLE;    
    EXTI_Init(&exti_initstruct);

    GPIO_EXTILineConfig(KEY3_EXTI_PORTSOURCE,KEY3_EXTI_PINSOURCE);    
    exti_initstruct.EXTI_Line       = KEY3_EXTI_LINE;
    exti_initstruct.EXTI_Mode       = EXTI_Mode_Interrupt;
    exti_initstruct.EXTI_Trigger    = EXTI_Trigger_Falling;
    exti_initstruct.EXTI_LineCmd    = ENABLE;    
    EXTI_Init(&exti_initstruct);
}


/* 配置NVIC中断，中断优先级和中断通道 */
void KEY_NVIC_Config(void)
{
    NVIC_InitTypeDef nvic_initstruct = {0};                                 /* 定义一个 NVIC 结构体 */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);                     /* 开启 AFIO 相关的时钟 */

    nvic_initstruct.NVIC_IRQChannel                     = KEY1_EXTI_IRQ;    /* 配置中断源 */
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  7;               /* 配置抢占优先级 */
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;               /* 配置子优先级 */
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;          /* 使能配置中断通道 */
    NVIC_Init(&nvic_initstruct);

    nvic_initstruct.NVIC_IRQChannel                     = KEY2_EXTI_IRQ;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  7;
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;
    NVIC_Init(&nvic_initstruct);

    nvic_initstruct.NVIC_IRQChannel                     = KEY3_EXTI_IRQ;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  7;
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;
    NVIC_Init(&nvic_initstruct);
}


/* 按键状态读取：通过key_info结构体获取当前按键的电平状态 */
static uint8_t Get_KeyCurrentLevel(KEY_Info* key_info)
{
    return GPIO_ReadInputDataBit(key_info->GPIOx,key_info->GPIO_Pin);
}

/* 获取按键默认电平，即上电后未按下时的电平 */
void KeyLevel_Init(KEY_Info* key_info)
{
    if(key_info->triggerlevel == KEY_GENERAL_TRIGGER)
    {
        key_info->triggerlevel = Get_KeyCurrentLevel(key_info)? KEY_LOW_TRIGGER:KEY_HIGH_TRIGGER;
    }
}


/* 按键初始化 */
void KEY_Init(void)
{
    KEY_GPIO_Config();              /* 对应的 GPIO 的配置 */
    KEY_Mode_Config();              /* 配置 KEY 模式 */
    KEY_NVIC_Config();              /* 配置 KEY 中断配置 */
    KeyLevel_Init(&key1_info);      /* 配置 KEY 初始电平 */
    KeyLevel_Init(&key2_info);      /* 配置 KEY 初始电平 */
    KeyLevel_Init(&key3_info);      /* 配置 KEY 初始电平 */
}


/* 按键扫描，判断按键是否被按下 */
KEY_Status KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KEY_TriggerLevel key_pressstatus)
{
    if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == key_pressstatus)
    {
        DWT_DelayMs(20);
        while(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == key_pressstatus);
        DWT_DelayMs(20);
        return KEY_DOWN;
    }
    else
    {
        return KEY_UP;
    }
}


/* 系统定时器扫描判断按键事件类型 */
KEY_Event KEY_SystickScan(KEY_Info* key_info)
{
    uint64_t time_time = 0;

    /* 检测当前按键状态 */
    if(Get_KeyCurrentLevel(key_info) ==  key_info->triggerlevel)
    {
        key_info->status = KEY_DOWN;
    }
    else
    {
        key_info->status = KEY_UP;
    }
    switch(key_info->event)
    {
        /* 空闲状态 */
        case EVENT_ATTONITY :
            if(key_info->status == KEY_DOWN) /* 检测到按下 */
            {
                key_info->event = EVENT_PRESS; /* 进入按下状态 */
                key_info->press_time = SysTick_GetCount(); /* 记录按下时间 */
            }
            break;
            
        /* 按下状态 */
        case EVENT_PRESS:
            if(key_info->status == KEY_DOWN) /* 仍然按下 */
            {
                time_time = SysTick_GetCount();
                if(time_time - key_info->press_time >1000) /* 如果按下超过1s，则为长按 */
                {
                    key_info->event = EVENT_LONG; /* 进入长按状态 */
                } 
            }
            else
            {
                key_info->event = EVENT_SHORT; /* 否则为短按 */
            }
            break;
                
        /* 短按状态 */
        case EVENT_SHORT:
            key_info->event = EVENT_SHORT_RELEASE; /* 短按释放 */
        break;
        
        /* 长按状态 */
        case EVENT_LONG:
            if(key_info->status == KEY_DOWN) /* 仍然长按 */
            {
                key_info->event = EVENT_LONG; /* 保持长按状态 */
            }
            else
            {
                key_info->event = EVENT_LONG_RELEASE; /* 长按释放 */
            }
            break;
        
        /* 其他状态 */
        default :
            key_info->event = EVENT_ATTONITY; /* 回到空闲状态 */
            break;        
    }
    return key_info->event; /* 返回当前事件 */
}

/*****************************END OF FILE***************************************/

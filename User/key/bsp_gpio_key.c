/**
  ******************************************************************************
  * @file       bsp_gpio_key.c
  * @author     embedfire
  * @version     V1.0
  * @date        2024
  * @brief      扫描按键函数接口
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

#include "key/bsp_gpio_key.h"
#include "systick/bsp_systick.h"
#include "dwt/bsp_dwt.h"

#include "FreeRTOS.h"
#include "task.h"

KEY_Info key1_info  = {KEY1_GPIO_PORT,KEY1_GPIO_PIN,KEY_GENERAL_TRIGGER,KEY_INIT,0,0,EVENT_ATTONITY,KEY_NONE_CLICK};
KEY_Info key2_info  = {KEY2_GPIO_PORT,KEY2_GPIO_PIN,KEY_GENERAL_TRIGGER,KEY_INIT,0,0,EVENT_ATTONITY,KEY_NONE_CLICK};


/* 配置按键中断的NVIC */
void KEY_NVIC_Config(void)
{
    /* 定义一个 NVIC 初始化结构体并清零 */
    NVIC_InitTypeDef nvic_initstruct = {0};
    
    /* 开启 AFIO 相关的时钟，用于配置外部中断线（GPIO重映射） */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
    
#if 1    
    /* 配置中断源，即按键1的中断通道 */
    nvic_initstruct.NVIC_IRQChannel                     = KEY1_EXTI_IRQ;
    /* 配置抢占优先级 */
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  7;
    /* 配置子优先级 */
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;
    /* 使能配置中断通道 */
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;

    NVIC_Init(&nvic_initstruct);
#endif
    
#if 1
    nvic_initstruct.NVIC_IRQChannel                     = KEY2_EXTI_IRQ;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority   =  7;
    nvic_initstruct.NVIC_IRQChannelSubPriority          =  0;
    nvic_initstruct.NVIC_IRQChannelCmd                  =  ENABLE;

    NVIC_Init(&nvic_initstruct);
#endif

}

/* 配置按键的GPIO */
void KEY_GPIO_Config(void)
{
    /* 定义一个 GPIO 结构体 */
    GPIO_InitTypeDef gpio_initstruct = {0};

#if 1    
    /* 开启按键1的GPIO时钟 */
    RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK_PORT,ENABLE);
    /* 设置引脚为高电平（上拉） */
    GPIO_SetBits(KEY1_GPIO_PORT,KEY1_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = KEY1_GPIO_PIN;            /* 设置引脚号 */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;    /* 浮空输入模式 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;         /* 引脚速度50Hz */
    GPIO_Init(KEY1_GPIO_PORT,&gpio_initstruct);             /* 应用GPIO配置 */
#endif 
    
#if 1
    RCC_APB2PeriphClockCmd(KEY2_GPIO_CLK_PORT,ENABLE);
    GPIO_SetBits(KEY2_GPIO_PORT,KEY2_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = KEY2_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(KEY2_GPIO_PORT,&gpio_initstruct);   
#endif 

}

/* 配置外部中断模式 */
void KEY_Mode_Config(void)
{
    /* 定义一个外部中断初始化结构体并清零 */
    EXTI_InitTypeDef exti_initstruct = {0};
    /* 开启AFIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
    
#if 1
    GPIO_EXTILineConfig(KEY1_EXTI_PORTSOURCE,KEY1_EXTI_PINSOURCE); /* GPIO与EXTI线映射 */
    exti_initstruct.EXTI_Line       = EXTI_Line0;           /* 使用EXTI线0 */
    exti_initstruct.EXTI_Mode       = EXTI_Mode_Interrupt;  /* 中断模式（非事件模式） */
    exti_initstruct.EXTI_Trigger    = EXTI_Trigger_Falling; /* 下降沿触发 */
    exti_initstruct.EXTI_LineCmd    = ENABLE;               /* 使能EXTI线 */
    EXTI_Init(&exti_initstruct);                            /* 应用EXTI配置 */
#endif
    
#if 1
    GPIO_EXTILineConfig(KEY2_EXTI_PORTSOURCE,KEY2_EXTI_PINSOURCE);
    exti_initstruct.EXTI_Line       = EXTI_Line13;
    exti_initstruct.EXTI_Mode       = EXTI_Mode_Interrupt;
    exti_initstruct.EXTI_Trigger    = EXTI_Trigger_Falling;
    exti_initstruct.EXTI_LineCmd    = ENABLE;
    EXTI_Init(&exti_initstruct);
#endif
}

void KEY_Init(void)
{
    KEY_NVIC_Config();              /* 配置中断控制器 */
    KEY_GPIO_Config();              /* 配置GPIO引脚 */
    KEY_Mode_Config();              /* 配置外部中断模式 */
    KeyLevel_Init(&key1_info);      /* 初始化按键1的电平状态 */
    KeyLevel_Init(&key2_info);      /* 初始化按键2的电平状态 */
}

/* 基础按键扫描函数，在FreeRTOS中未被使用，因为其调用的延时函数会阻塞FreeRTOS系统 */
KEY_Status KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KEY_TriggerLevel key_pressstatus)
{
    /* 检测按键是否按下 */
    if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == key_pressstatus)
    {
        DWT_Delay_Ms(20);
        while(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == key_pressstatus);
        DWT_Delay_Ms(20);
        return KEY_DOWN;
    }
    else
    {
        return KEY_UP;
    }
}

/* 获取当前电平 */
static uint8_t Get_KeyCurrentLevel(KEY_Info* key_info)
{
    return GPIO_ReadInputDataBit(key_info->GPIOx,key_info->GPIO_Pin);
}

/* 初始化按键电平状态 */
void KeyLevel_Init(KEY_Info* key_info)
{
    if(key_info->triggerlevel == KEY_GENERAL_TRIGGER)
    {
        /* 上电后程序启动短时间内默认按键未按，那么根据当前未按下电平，反推出按下的电平 */
        key_info->triggerlevel = Get_KeyCurrentLevel(key_info)? KEY_LOW_TRIGGER:KEY_HIGH_TRIGGER;
    }

}

/* 使用状态机实现高级按键检测，支持短按、长按等复杂事件，适合在FreeRTOS任务中周期性使用 */
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
                key_info->press_time = xTaskGetTickCount(); /* 记录按下时间 */
            }
            break;
            
        /* 按下状态 */
        case EVENT_PRESS:
            if(key_info->status == KEY_DOWN) /* 仍然按下 */
            {
                time_time = xTaskGetTickCount();
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

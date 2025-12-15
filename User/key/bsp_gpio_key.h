#ifndef __BSP_GPIO_KEY_H
#define __BSP_GPIO_KEY_H

#include "stm32f10x.h"

//KEY1
#define KEY1_GPIO_PORT          GPIOA                           /* GPIO端口 */
#define KEY1_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
#define KEY1_GPIO_PIN           GPIO_Pin_0                      /* 对应PIN脚 */

#define KEY1_EXTI_PORTSOURCE    GPIO_PortSourceGPIOA            /* 中断端口源 */
#define KEY1_EXTI_PINSOURCE     GPIO_PinSource0                 /* 中断PIN源 */
#define KEY1_EXTI_LINE          EXTI_Line0                      /* 中断线 */
#define KEY1_EXTI_IRQ           EXTI0_IRQn                      /* 外部中断向量号 */
#define KEY1_EXTI_IRQHANDLER    EXTI0_IRQHandler                /* 中断处理函数 */

//KEY2
#define KEY2_GPIO_PORT          GPIOC                           /* GPIO端口 */
#define KEY2_GPIO_CLK_PORT      RCC_APB2Periph_GPIOC            /* GPIO端口时钟 */
#define KEY2_GPIO_PIN           GPIO_Pin_13                     /* 对应PIN脚 */

#define KEY2_EXTI_PORTSOURCE    GPIO_PortSourceGPIOC            /* 中断端口源 */
#define KEY2_EXTI_PINSOURCE     GPIO_PinSource13                /* 中断PIN源 */
#define KEY2_EXTI_LINE          EXTI_Line13                     /* 中断线 */
#define KEY2_EXTI_IRQ           EXTI15_10_IRQn                  /* 外部中断向量号 */
#define KEY23_EXTI_IRQHANDLER   EXTI15_10_IRQHandler            /* 中断处理函数 */

//KEY3
#define KEY3_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define KEY3_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define KEY3_GPIO_PIN           GPIO_Pin_15                     /* 对应PIN脚 */

#define KEY3_EXTI_PORTSOURCE    GPIO_PortSourceGPIOB            /* 中断端口源 */
#define KEY3_EXTI_PINSOURCE     GPIO_PinSource15                /* 中断PIN源 */
#define KEY3_EXTI_LINE          EXTI_Line15                     /* 中断线 */
#define KEY3_EXTI_IRQ           EXTI15_10_IRQn                  /* 外部中断向量号 */


/* 按键按下时的IO电平 */
typedef enum 
{
    KEY_LOW_TRIGGER = 0,        /* 低电平触发 */
    KEY_HIGH_TRIGGER = 1,       /* 高电平触发 */
    KEY_GENERAL_TRIGGER = 2,    /* 默认值 */
}KEY_TriggerLevel;

/* 按键的状态 */
typedef enum 
{
    KEY_UP = 0,                 /* 按键未被按下 */
    KEY_DOWN = 1,               /* 按键被按下 */
    KEY_INIT = 2,               /* 按键初始化状态 */
}KEY_Status;

/* 按键的事件类型 */
typedef enum 
{
    EVENT_ATTONITY = 0,         /* 默认状态：按键未被按下 */
    EVENT_PRESS = 1,            /* 按键被按下 */
    EVENT_SHORT,                /* 短按 */
    EVENT_SHORT_RELEASE,        /* 短按释放 */
    EVENT_LONG,                 /* 长按 */
    EVENT_LONG_RELEASE,         /* 长按释放 */
}KEY_Event;

/* 按键的点击类型 */
typedef enum 
{
    KEY_NONE_CLICK = 0,         /* 无点击 */
    KEY_SINGLE_CLICK = 1,       /* 单击 */
    KEY_DOUBLE_CLICK,           /* 双击 */
    KEY_LONG_CLICK,             /* 长按 */
}KEY_ClickType;

/* 按键的结构体 */
typedef struct
{
    GPIO_TypeDef*       GPIOx;          /* 按键对应的GPIO端口 */
    uint16_t            GPIO_Pin;       /* 按键对应的GPIO引脚 */
    KEY_TriggerLevel    triggerlevel;   /* 按键对应的触发电平 */
    KEY_Status          status;         /* 按键的状态 */
    uint64_t            press_time;     /* 按键按下的时间 */
    uint64_t            release_time;   /* 按键释放的时间 */
    KEY_Event           event;          /* 按键的事件类型 */
    KEY_ClickType       clicktype;      /* 按键的点击类型 */
}KEY_Info;

extern KEY_Info key1_info;
extern KEY_Info key2_info;
extern KEY_Info key3_info;

void KEY_NVIC_Config(void);
void KEY_GPIO_Config(void);
void KEY_Init(void);
KEY_Status KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KEY_TriggerLevel key_pressstatus);
void KeyLevel_Init(KEY_Info* key_info);
KEY_Event KEY_SystickScan(KEY_Info* key_info);

#endif /* __BSP_GPIO_KEY_H */

#ifndef __BSP_GPIO_KEY_H
#define __BSP_GPIO_KEY_H

#include "stm32f10x.h"

/* 定义 KEY 连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的 KEY 引脚 */

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
#define KEY2_EXTI_IRQHANDLER    EXTI15_10_IRQHandler            /* 中断处理函数 */


//KEY3
#define KEY3_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define KEY3_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define KEY3_GPIO_PIN           GPIO_Pin_15                     /* 对应PIN脚 */

//KEY4
#define KEY4_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define KEY4_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define KEY4_GPIO_PIN           GPIO_Pin_6                      /* 对应PIN脚 */

//KEY5
#define KEY5_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define KEY5_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define KEY5_GPIO_PIN           GPIO_Pin_7                      /* 对应PIN脚 */

//KEY6
#define KEY6_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define KEY6_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define KEY6_GPIO_PIN           GPIO_Pin_8                      /* 对应PIN脚 */


/* 按键按下时的IO电平 */
typedef enum 
{
    KEY_LOW_TRIGGER = 0, 
    KEY_HIGH_TRIGGER = 1,
    KEY_GENERAL_TRIGGER = 2,
}KEY_TriggerLevel;

/* 按键的状态 */
typedef enum 
{
    KEY_UP = 0, 
    KEY_DOWN = 1,
    KEY_INIT = 2,
}KEY_Status;

/* 按键的事件类型 */
typedef enum 
{
    EVENT_ATTONITY = 0, 
    EVENT_PRESS = 1,
    EVENT_SHORT,
    EVENT_SHORT_RELEASE,
    EVENT_LONG,
    EVENT_LONG_RELEASE,
}KEY_Event;

/* 按键的点击类型 */
typedef enum 
{
    KEY_NONE_CLICK = 0, 
    KEY_SINGLE_CLICK = 1,
    KEY_DOUBLE_CLICK,
    KEY_LONG_CLICK,
}KEY_ClickType;

/* 按键的结构体 */
typedef struct
{
    GPIO_TypeDef*       GPIOx;
    uint16_t            GPIO_Pin;
    KEY_TriggerLevel    triggerlevel;
    KEY_Status          status;
    uint32_t            press_time;
    uint32_t            release_time;
    KEY_Event           event;  
    KEY_ClickType       clicktype;
}KEY_Info;

extern KEY_Info key1_info;
extern KEY_Info key2_info;

void KEY_NVIC_Config(void);
void KEY_GPIO_Config(void);
void KEY_Mode_Config(void);
void KEY_Init(void);
KEY_Status KEY_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KEY_TriggerLevel key_pressstatus);
void KeyLevel_Init(KEY_Info* key_info);
KEY_Event KEY_SystickScan(KEY_Info* key_info);
#endif /* __BSP_GPIO_KEY_H */

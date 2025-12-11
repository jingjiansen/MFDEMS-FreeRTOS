#ifndef __BSP_GPIO_LED_H
#define __BSP_GPIO_LED_H

#include "stm32f10x.h"

/* 定义 LED 连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的LED引脚 */

//LED1
#define LED1_GPIO_PORT          GPIOA                           /* GPIO端口 */
#define LED1_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
#define LED1_GPIO_PIN           GPIO_Pin_1                      /* 对应PIN脚 */

//LED2
#define LED2_GPIO_PORT          GPIOA                           /* GPIO端口 */
#define LED2_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
#define LED2_GPIO_PIN           GPIO_Pin_2                      /* 对应PIN脚 */

//LED3
#define LED3_GPIO_PORT          GPIOA                           /* GPIO端口 */
#define LED3_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
#define LED3_GPIO_PIN           GPIO_Pin_3                      /* 对应PIN脚 */

//LED4
#define LED4_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define LED4_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define LED4_GPIO_PIN           GPIO_Pin_5                      /* 对应PIN脚 */

//LED5
#define LED5_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define LED5_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define LED5_GPIO_PIN           GPIO_Pin_13                      /* 对应PIN脚 */

//LED6
#define LED6_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define LED6_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define LED6_GPIO_PIN           GPIO_Pin_14                      /* 对应PIN脚 */

/**************************** 核心板载LED灯 *****************************/   
// R_LED
#define R_LED_GPIO_PORT          LED1_GPIO_PORT                /* GPIO端口 */
#define R_LED_GPIO_CLK_PORT      LED1_GPIO_CLK_PORT            /* GPIO端口时钟 */
#define R_LED_GPIO_PIN           LED1_GPIO_PIN                 /* 对应PIN脚 */

// G_LED  
#define G_LED_GPIO_PORT          LED2_GPIO_PORT                /* GPIO端口 */
#define G_LED_GPIO_CLK_PORT      LED2_GPIO_CLK_PORT            /* GPIO端口时钟 */
#define G_LED_GPIO_PIN           LED2_GPIO_PIN                 /* 对应PIN脚 */

// B_LED  
#define B_LED_GPIO_PORT          LED3_GPIO_PORT                /* GPIO端口 */
#define B_LED_GPIO_CLK_PORT      LED3_GPIO_CLK_PORT            /* GPIO端口时钟 */
#define B_LED_GPIO_PIN           LED3_GPIO_PIN                 /* 对应PIN脚 */

/************************************用户自定义应用宏*****************************************************/

// R_LED
#define R_LED_ON_ONLY    LED_ON(R_LED_GPIO_PORT,R_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_OFF(G_LED_GPIO_PORT,G_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_OFF(B_LED_GPIO_PORT,B_LED_GPIO_PIN,LED_LOW_TRIGGER);
// G_LED                     
#define G_LED_ON_ONLY   LED_OFF(R_LED_GPIO_PORT,R_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                         LED_ON(G_LED_GPIO_PORT,G_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_OFF(B_LED_GPIO_PORT,B_LED_GPIO_PIN,LED_LOW_TRIGGER);
// B_LED
#define B_LED_ON_ONLY   LED_OFF(R_LED_GPIO_PORT,R_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_OFF(G_LED_GPIO_PORT,G_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                         LED_ON(B_LED_GPIO_PORT,B_LED_GPIO_PIN,LED_LOW_TRIGGER);
                         
// R_G_B_LED 全亮
#define RGB_ALL_ON      LED_ON(R_LED_GPIO_PORT,R_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_ON(G_LED_GPIO_PORT,G_LED_GPIO_PIN,LED_LOW_TRIGGER);    \
                        LED_ON(B_LED_GPIO_PORT,B_LED_GPIO_PIN,LED_LOW_TRIGGER);
                         
// R_G_B_LED 全灭
#define RGB_ALL_OFF     LED_OFF(R_LED_GPIO_PORT,R_LED_GPIO_PIN,LED_LOW_TRIGGER);   \
                        LED_OFF(G_LED_GPIO_PORT,G_LED_GPIO_PIN,LED_LOW_TRIGGER);   \
                        LED_OFF(B_LED_GPIO_PORT,B_LED_GPIO_PIN,LED_LOW_TRIGGER);

/* LED灯亮时的IO电平 */
typedef enum 
{
    LED_LOW_TRIGGER = 0, 
    LED_HIGH_TRIGGER = 1,
}LED_TriggerLevel;

void LED_GPIO_Config(void);
void LED_ON(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_TriggerLevel led_brightstatus);
void LED_OFF(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_TriggerLevel led_brightstatus);
void LED_TOGGLE(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#endif /* __BSP_GPIO_LED_H */

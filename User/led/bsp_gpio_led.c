#include "led/bsp_gpio_led.h"

/* 初始化GPIO配置 */
void LED_GPIO_Config(void)
{
    
    GPIO_InitTypeDef gpio_initstruct = {0};             /* 定义一个 GPIO 结构体 */

    RCC_APB2PeriphClockCmd(R_LED_GPIO_CLK_PORT,ENABLE); /* 开启 LED 相关的GPIO外设/端口时钟 */
    GPIO_SetBits(R_LED_GPIO_PORT,R_LED_GPIO_PIN);       /* IO输出状态初始化控制 */
    gpio_initstruct.GPIO_Pin    = R_LED_GPIO_PIN;       /* 选择要控制的GPIO引脚 */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;     /* 设置GPIO模式为 推挽模式 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;     /* 设置GPIO速率为50MHz */
    GPIO_Init(R_LED_GPIO_PORT,&gpio_initstruct);        /* 初始化GPIO */
   
    RCC_APB2PeriphClockCmd(G_LED_GPIO_CLK_PORT,ENABLE);    
    GPIO_SetBits(G_LED_GPIO_PORT,G_LED_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = G_LED_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(G_LED_GPIO_PORT,&gpio_initstruct);

    RCC_APB2PeriphClockCmd(B_LED_GPIO_CLK_PORT,ENABLE);
    GPIO_SetBits(B_LED_GPIO_PORT,B_LED_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = B_LED_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(B_LED_GPIO_PORT,&gpio_initstruct);
    
    RCC_APB2PeriphClockCmd(LED4_GPIO_CLK_PORT,ENABLE);
    GPIO_SetBits(LED4_GPIO_PORT,LED4_GPIO_PIN);
    gpio_initstruct.GPIO_Pin    = LED4_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(LED4_GPIO_PORT,&gpio_initstruct);
}


/* 开灯 */
void LED_ON(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_TriggerLevel led_brightstatus)
{
    if(led_brightstatus == LED_LOW_TRIGGER)
    {
        GPIO_ResetBits(GPIOx,GPIO_Pin);
    }
    else
    {
        GPIO_SetBits(GPIOx,GPIO_Pin);
    }    
}

/* 关灯 */
void LED_OFF(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_TriggerLevel led_brightstatus)
{
    if(led_brightstatus == LED_LOW_TRIGGER)
    {
        GPIO_SetBits(GPIOx,GPIO_Pin);
    }
    else
    {
        GPIO_ResetBits(GPIOx,GPIO_Pin);
    }
}

/* 翻转灯 */
void LED_TOGGLE(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;

}

/*****************************END OF FILE***************************************/

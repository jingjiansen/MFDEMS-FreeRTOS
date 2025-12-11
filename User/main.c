/* FreeRTOS 相关头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* 开发板硬件相关头文件 */
#include "usart/bsp_usart.h"        /* 串口打印 */
#include "usart/usart_com.h"        /* 串口打印 */
#include "led/bsp_gpio_led.h"       /* LED灯 */
#include "key/bsp_gpio_key.h"       /* 按键 */

#include <string.h>

static TaskHandle_t LED_Task_Handle = NULL;
static TaskHandle_t Receive_Task_Handle = NULL;

QueueHandle_t Test_Queue = NULL;
SemaphoreHandle_t BinarySem_Handle = NULL;

#define QUEUE_LEN 4
#define QUEUE_SIZE 4

/************************************ 函数声明 ***************************************/
static void LED_Task(void *pvParameters); /* LED1任务，负责点亮红灯 */
static void Receive_Task(void *pvParameters);  /* 接收任务，负责接收串口数据 */
static void BSP_Init(void); /* BSP初始化 */


/* 主函数 */
int main(void)
{
    BSP_Init();
    
    BaseType_t xReturn1 = pdPASS;
    BaseType_t xReturn2 = pdPASS;

    taskENTER_CRITICAL();

    Test_Queue = xQueueCreate((UBaseType_t) QUEUE_LEN,
                              (UBaseType_t) QUEUE_SIZE); 
    if(Test_Queue != NULL)
        printf("消息队列创建成功！\n");
    else
        printf("消息队列创建失败！\n");

    BinarySem_Handle = xSemaphoreCreateBinary();
    if(BinarySem_Handle != NULL)
        printf("二进制信号量创建成功！\n");
    else
        printf("二进制信号量创建失败！\n");

    
    xReturn1 = xTaskCreate(
        (TaskFunction_t) LED_Task,             /*任务函数*/
        (const char *) "LED_Task",             /*任务名称*/
        (uint32_t) 512,           /*任务堆栈大小*/
        (void*) NULL,                           /*传递给任务函数的参数*/
        (UBaseType_t) 2,                        /*任务优先级*/
        (TaskHandle_t *)&LED_Task_Handle       /*任务控制块*/
    );
    if(xReturn1 == pdPASS) 
        printf("LED_Task task create done\n");
    else
        printf("LED_Task task create failed\n");

        
    xReturn2 = xTaskCreate(
        (TaskFunction_t) Receive_Task,             /*任务函数*/
        (const char *) "Receive_Task",             /*任务名称*/
        (uint32_t) 512,           /*任务堆栈大小*/
        (void*) NULL,                           /*传递给任务函数的参数*/
        (UBaseType_t) 3,                        /*任务优先级*/
        (TaskHandle_t *)&Receive_Task_Handle       /*任务控制块*/
    );
    if(xReturn2 == pdPASS) 
        printf("KEY_Task task create done\n");
    else 
        printf("KEY_Task task create failed\n");


    if(xReturn1 == pdPASS && xReturn2 == pdPASS)
    {
        vTaskStartScheduler(); /*启动任务，开始调度 */
    }
    
    while(1); /* 正常不会执行到这里 */
}


/**
 * @brief 初始化板级支持包（BSP）
 * @note 初始化包括时钟、GPIO等，所有板子上的初始化均可放在这个函数中
 */
static void BSP_Init(void)
{
    /*
     * SMT32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0-15
     * 优先级分组只需要分组一次，后续如果其他任务需要用到中断，都统一用这个分组，不可再次分组
    */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 打印串口初始化 */
    USART_INIT();
    printf("usart ok\n");

    /* LED灯初始化 */
    LED_GPIO_Config();
    printf("led gpio ok\n");
    
    /* KEY配置：中断、GPIO、模式、按键初始状态 */
    KEY_Init();
    printf("key ok.\n");

    printf("bsp init done!\n");
}


/**
 * @brief LED1任务，用于控制红灯闪烁
 * @param None
 * @retval None
 */
static void LED_Task(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;
    uint32_t r_queue;

    while(1)
    {
        xReturn = xQueueReceive(Test_Queue, &r_queue, portMAX_DELAY);
        if(xReturn == pdPASS)
            printf("触发中断的是 KEY%d !\n", r_queue);
        else
            printf("数据接收出错\n");
    }
}

/* 用于接收来自串口的消息，并将其通过printf回传 */
static void Receive_Task(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;
    while(1)
    {
        xReturn = xSemaphoreTake(BinarySem_Handle, portMAX_DELAY);
        if(xReturn == pdPASS)
        {
            /* 使用 bsp_debug 中的 debug_receive 缓冲区显示并清理 */
            printf("接收数据：%s \n", usart_receive.buffer);
            /* 清空接收缓存并重置状态，允许下次接收 */
            memset(usart_receive.buffer, 0, sizeof(usart_receive.buffer)); /* 清空接收缓存 */
            usart_receive.len = 0;
            usart_receive.read_flag = 0;
        }
    }
}

/************************************END OF FILE ****************************************/

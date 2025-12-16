#include "dht11/app_dht11.h"
#include "dht11/bsp_dht11.h" 
#include "usart/usart_com.h"
#include "debug/bsp_debug.h"
#include "oled/app_oled.h"
#include "led/bsp_gpio_led.h"

#include "FreeRTOS.h"
#include "task.h"

QueueHandle_t xDht11Queue = NULL;

/* DHT11 初始化*/
void DHT11_Init(void) 
{
    /* 创建数据队列，深度为1，使用覆盖模式 */
    xDht11Queue = xQueueCreate(1,sizeof(DHT11_DATA_TYPEDEF));
}


/**
 * @brief  DHT11任务  
 * @param  无
 * @retval 无
 */
void DHT11_Task(void *pvParameters) 
{
    DHT11_DATA_TYPEDEF last_valid_data = {0};

    uart_printf("DHT11 task start.\r\n");

    while(1) 
    {
        if(DHT11_ReadData(&last_valid_data) == SUCCESS) 
        {
            /* 发送数据到队列 */
            if(xQueueOverwrite(xDht11Queue, &last_valid_data) == pdTRUE) 
            {
                // uart_printf("DHT11数据更新：");
                // if(last_valid_data.humi_deci&0x80) 
                // {
                //     uart_printf("湿度为 -%d.%d％ＲＨ ",last_valid_data.humi_int,last_valid_data.humi_deci);
                // }
                // else 
                // {
                //     uart_printf("湿度为 %d.%d％ＲＨ ",last_valid_data.humi_int,last_valid_data.humi_deci);
                // }

                // if(last_valid_data.temp_deci&0x80) 
                // {
                //     uart_printf("温度为 -%d.%d ℃ \r\n",last_valid_data.temp_int,last_valid_data.temp_deci);
                // }
                // else 
                // {
                //     uart_printf("温度为 %d.%d ℃ \r\n",last_valid_data.temp_int,last_valid_data.temp_deci);
                // }
            }
            else 
            {
                uart_printf("DHT11数据队列发送错误!\r\n");
            }
        }
        else 
        {
            uart_printf("DHT11数据读取错误!\r\n");
        }        
        vTaskDelay(500);
    }
}

/*****************************END OF FILE***************************************/

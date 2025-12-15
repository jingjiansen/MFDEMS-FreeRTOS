#include "main.h" /* for include dht11 task and oled task */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "debug/bsp_debug.h"
#include "led/bsp_gpio_led.h"
#include "key/bsp_gpio_key.h"

static TaskHandle_t Display_Task_Handler = NULL;
static TaskHandle_t DHT11_Task_Handler = NULL;

int main(void)
{
    Bsp_Init();
    App_Init();
    
    BaseType_t ret1 = pdPASS;
    BaseType_t ret2 = pdPASS;

    
    /* 创建菜单任务 */
    ret1 = xTaskCreate(  (TaskFunction_t) Display_Task,
                            (const char*) "Display_Task",
                            (uint32_t) 512,
                            (void*) NULL,
                            (UBaseType_t) 2,
                            (TaskHandle_t *) &Display_Task_Handler
                         );
    /* 创建DHT11任务 */
    ret1 = xTaskCreate(  (TaskFunction_t) DHT11_Task,
                            (const char*) "DHT11_Task",
                            (uint32_t) 512,
                            (void*) NULL,
                            (UBaseType_t) 3,
                            (TaskHandle_t *) &DHT11_Task_Handler
                         );

     if(ret1 == pdPASS && ret2 == pdPASS)
     {
         vTaskStartScheduler();
     }
     
     while(1);              
}

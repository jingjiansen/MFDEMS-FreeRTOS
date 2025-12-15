#ifndef __APP_DHT11_H
#define __APP_DHT11_H

#include "stm32f10x.h"
#include "dht11/bsp_dht11.h" 

#include "FreeRTOS.h"
#include "queue.h"
extern QueueHandle_t xDht11Queue;

void DHT11_Init(void);
void DHT11_Task(void *pvParameters);

#endif /* __APP_DHT11_H  */


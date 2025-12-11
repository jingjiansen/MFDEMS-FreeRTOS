#ifndef __BSP_DHT11_H
#define __BSP_DHT11_H

#include "stm32f10x.h"

/* 定义 DHT11_DATA 连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的 DHT11_DATA 引脚 */

//DHT11_DATA
#define DHT11_DATA_GPIO_PORT          GPIOB                           /* GPIO端口 */
#define DHT11_DATA_GPIO_CLK_PORT      RCC_APB2Periph_GPIOB            /* GPIO端口时钟 */
#define DHT11_DATA_GPIO_PIN           GPIO_Pin_12                     /* 对应PIN脚 */


#define DHT11_DATA_IN()         GPIO_ReadInputDataBit(DHT11_DATA_GPIO_PORT,DHT11_DATA_GPIO_PIN)
#define DHT11_DATA_OUT(VALUE)   if(VALUE)   GPIO_SetBits(DHT11_DATA_GPIO_PORT,DHT11_DATA_GPIO_PIN);\
                                else      GPIO_ResetBits(DHT11_DATA_GPIO_PORT,DHT11_DATA_GPIO_PIN)

                                
typedef struct
{                            
   uint8_t humi_int;        //湿度的整数部分
   uint8_t humi_deci;       //湿度的小数部分
   uint8_t temp_int;        //温度的整数部分
   uint8_t temp_deci;       //温度的小数部分
   uint8_t check_sum;       //校验和                              
}DHT11_DATA_TYPEDEF;                               
                                
void DHT11_GPIO_Config(void);
void DHT11_DataPinModeConfig(GPIOMode_TypeDef mode);
uint8_t DHT11_ReadByte(void);
ErrorStatus DHT11_ReadData(DHT11_DATA_TYPEDEF *dht11_data);

#endif /* __BSP_DHT11_H */

#ifndef __BSP_I2C_H
#define	__BSP_I2C_H

#include "stm32f10x.h"

#define IIC_NUM 1

#if (IIC_NUM == 1)

    #define IIC_I2CX                            I2C1
    #define IIC_I2CX_CLK_PORT                   RCC_APB1Periph_I2C1
    
    #define IIC_SCL_GPIO_PORT    			    GPIOB			                /* 对应GPIO端口 */
    #define IIC_SCL_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
    #define IIC_SCL_GPIO_PIN			        GPIO_Pin_6	       				/* 对应PIN脚 */

    #define IIC_SDA_GPIO_PORT    			    GPIOB			                /* 对应GPIO端口 */
    #define IIC_SDA_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
    #define IIC_SDA_GPIO_PIN			        GPIO_Pin_7	       				/* 对应PIN脚 */

#elif (IIC_NUM == 2)

    #define IIC_I2CX                            I2C2
    #define IIC_I2CX_CLK_PORT                   RCC_APB1Periph_I2C2
    
    #define IIC_SCL_GPIO_PORT    			    GPIOB			                /* 对应GPIO端口 */
    #define IIC_SCL_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
    #define IIC_SCL_GPIO_PIN			        GPIO_Pin_10	       				/* 对应PIN脚 */

    #define IIC_SDA_GPIO_PORT    			    GPIOB			                /* 对应GPIO端口 */
    #define IIC_SDA_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
    #define IIC_SDA_GPIO_PIN			        GPIO_Pin_11	       				/* 对应PIN脚 */

#endif

#define IIC_SDA_IN()           GPIO_ReadInputDataBit(IIC_SDA_GPIO_PORT,IIC_SDA_GPIO_PIN)  

#define IIC_DATA_READ          IIC_SDA_IN() 

#define IIC_SCL_OUT(VALUE)     (VALUE) ? GPIO_SetBits(IIC_SCL_GPIO_PORT,IIC_SCL_GPIO_PIN):GPIO_ResetBits(IIC_SCL_GPIO_PORT,IIC_SCL_GPIO_PIN)
#define IIC_SDA_OUT(VALUE)     (VALUE) ? GPIO_SetBits(IIC_SDA_GPIO_PORT,IIC_SDA_GPIO_PIN):GPIO_ResetBits(IIC_SDA_GPIO_PORT,IIC_SDA_GPIO_PIN)


/* 这个地址只要与外挂的I2C器件地址不一样即可 */
#define IIC_I2CX_OWN_ADDRESS7      0x0A  

/* IIC 速度模式 */  
#define IIC_SPEED              400000

/*等待超时时间*/
#define CHECK_TIMES         10

typedef enum
{
    IIC_WRITE = 0,
    IIC_READ = 1
}IIC_Direction_TypeDef;

void IIC_PinConfig(void);
void IIC_Init(void);
void IIC_DELAY_US(uint32_t time);
ErrorStatus IIC_Start(void);
void IIC_Stop(void);
ErrorStatus IIC_AddressMatching(uint8_t slave_addr,IIC_Direction_TypeDef direction);
ErrorStatus IIC_SendData(uint8_t data);
ErrorStatus IIC_CheckDevice(uint8_t slave_addr);

#endif /* __BSP_I2C_H  */
                                        

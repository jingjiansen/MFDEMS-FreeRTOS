#ifndef __BSP_SOFTI2C_H
#define	__BSP_SOFTI2C_H

#include "stm32f10x.h"

/*SCL时钟总线使用GPIOB6引脚*/
#define SOFT_IIC_SCL_GPIO_PORT    			    GPIOB			                /* 对应GPIO端口 */
#define SOFT_IIC_SCL_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
#define SOFT_IIC_SCL_GPIO_PIN			        GPIO_Pin_6	       				/* 对应PIN脚 */

/*SDA数据总线使用GPIOB7引脚*/
#define SOFT_IIC_SDA_GPIO_PORT    			    GPIOB       	                /* 对应GPIO端口 */
#define SOFT_IIC_SDA_GPIO_CLK_PORT 	            RCC_APB2Periph_GPIOB			/* 对应GPIO端口时钟位 */
#define SOFT_IIC_SDA_GPIO_PIN			        GPIO_Pin_7	       				/* 对应PIN脚 */

/*读取数据总线上的数据*/
#define SOFT_IIC_SDA_IN() GPIO_ReadInputDataBit(SOFT_IIC_SDA_GPIO_PORT, SOFT_IIC_SDA_GPIO_PIN)

/*SCL输出电平，可输出高低电平分别代表1和0*/
#define SOFT_IIC_SCL_OUT(VALUE) if(VALUE)   GPIO_SetBits(SOFT_IIC_SCL_GPIO_PORT, SOFT_IIC_SCL_GPIO_PIN); \
                                else           GPIO_ResetBits(SOFT_IIC_SCL_GPIO_PORT, SOFT_IIC_SCL_GPIO_PIN);

/*SDA输出电平，可输出高低电平分别代表1和0*/
#define SOFT_IIC_SDA_OUT(VALUE) if(VALUE)   GPIO_SetBits(SOFT_IIC_SDA_GPIO_PORT, SOFT_IIC_SDA_GPIO_PIN); \
                                else           GPIO_ResetBits(SOFT_IIC_SDA_GPIO_PORT, SOFT_IIC_SDA_GPIO_PIN);

/*进一步重命名读取数据总线数值函数*/
#define SOFT_IIC_DATA_READ SOFT_IIC_SDA_IN()

/*应答信号枚举*/
typedef enum
{
    SOFT_IIC_ACK = 0,
    SOFT_IIC_NACK = 1,
}SOFT_IIC_Respond_TypeDef;

/*发送方向（读写）枚举*/
typedef enum
{
    SOFT_IIC_WRITE = 0,
    SOFT_IIC_READ = 1,
}SOFT_IIC_Direction_TypeDef;

void SOFT_IIC_GPIO_Config(void);
void SOFT_IIC_Init(void);
void SOFT_IIC_DELAY_US(uint32_t time);
void SOFT_IIC_Start(void);
void SOFT_IIC_Stop(void);
uint8_t SOFT_IIC_WaitAck(void);
ErrorStatus SOFT_IIC_SendData(uint8_t data);
ErrorStatus SOFT_IIC_AddressMatching(uint8_t slave_addr,SOFT_IIC_Direction_TypeDef direction);
ErrorStatus SOFT_IIC_CheckDevice(uint8_t slave_addr);


#endif /* __BSP_SOFTI2C_H  */

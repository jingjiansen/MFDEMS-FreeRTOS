#include "i2c/bsp_softi2c.h"
#include "dwt/bsp_dwt.h"        /* 用于延时 */

uint32_t i2c_delay_time = 1;    /* 设置开始信号之前的默认状态的保持时间 */


/* 使用PB6和PB7分别作为IIC的SCL和SDA总线，即分别为时钟线和数据线 */
/* 只要GPIO引脚可以翻转电平就可以模拟IIC */
void SOFT_IIC_GPIO_Config(void)
{
    GPIO_InitTypeDef gpio_initstruct = {0};

    /* 配置SCL总线的GPIO */
    RCC_APB2PeriphClockCmd(SOFT_IIC_SCL_GPIO_CLK_PORT,ENABLE); /* 开启GPIO所使用的时钟，这里使用的是APB2总线*/
    SOFT_IIC_SCL_OUT(1);                                       /* 设置时钟总线为高电平（初始状态） */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_OD;            /* 设置为开漏输出模式 */
    gpio_initstruct.GPIO_Pin    = SOFT_IIC_SCL_GPIO_PIN;       /* 设置为SCL引脚PB6 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;            /* 设置输出速度为50MHz */
    GPIO_Init(SOFT_IIC_SCL_GPIO_PORT,&gpio_initstruct); 
    
    /*配置SDA总线的GPIO*/
    RCC_APB2PeriphClockCmd(SOFT_IIC_SDA_GPIO_CLK_PORT,ENABLE); /*开启GPIO所使用的时钟，这里使用的是APB2总线*/
    SOFT_IIC_SDA_OUT(1);                                       /* 设置数据总线为高电平（初始状态） */
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_Out_OD;            /* 设置为开漏输出模式 */
    gpio_initstruct.GPIO_Pin    = SOFT_IIC_SDA_GPIO_PIN;       /* 设置为SDA引脚PB7 */
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;            /* 设置输出速度为50MHz */
    GPIO_Init(SOFT_IIC_SDA_GPIO_PORT,&gpio_initstruct);
}


/* SOFT_IIC初始化 */
void SOFT_IIC_Init(void)
{
    SOFT_IIC_GPIO_Config(); /* 配置IIC所用到的GPIO*/
}


/* 延时接口：用于维持高低电平 */
void SOFT_IIC_DELAY_US(uint32_t time)
{
    DWT_DelayUs(time); /* 使用DWT的精确延时 */
}


/* 模拟IIC的开始信号 */
void SOFT_IIC_Start(void)
{
    /* 当SCL为高电平时，SDA出现一个下降沿（即由高电平转为低电平）时为开始信号 */
    SOFT_IIC_SDA_OUT(1); /* 设置数据总线为高电平 */
    SOFT_IIC_SCL_OUT(1); /* 设置时钟总线为高电平 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 延时 */
    SOFT_IIC_SDA_OUT(0); /* 设置数据总线为低电平，即数据总线产生下降沿 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 延时 */
    /* 在开始信号发送之后，拉低时钟总线，如果保持时钟总线为高电平，
     * 如果后续发送的数据为高电平，则会产生一个停止信号；
     * 如果后续发送的数据为低电平，则会额外产生一次开始信号。
     * OLED屏幕的地址为0x3C（00111100），首个bit为0，因此在发送地址数据，其首bit被当成了开始信号，
     * 需要先拉低SCL总线，这样才能捕捉到SDA总线上的数据内容。
    */
    SOFT_IIC_SCL_OUT(0); /* 时钟总线返回低电平 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 延时 */
}


/* 模拟IIC的停止信号 */
void SOFT_IIC_Stop(void)
{
    /* 当SCL为高电平时，SDA出现一个上升沿（即由低电平转为高电平）时为停止信号 */
    SOFT_IIC_SDA_OUT(0); /* 设置数据总线为低电平 */
    SOFT_IIC_SCL_OUT(1); /* 设置时钟总线为高电平 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /*延时*/
    SOFT_IIC_SDA_OUT(1); /* 数据总线返回高电平 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 延时 */
}


/* 产生一个时钟，并读取从机的ACK应答信号 */
uint8_t SOFT_IIC_WaitAck(void)
{
    uint8_t ret = SOFT_IIC_NACK; /* 初始化应答信号为NACK（非应答） */
    SOFT_IIC_SDA_OUT(1); /* 释放SDA总线 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 释放之后等待一段时间（让对方输出应答信号），读取应答信号 */
    
    SOFT_IIC_SCL_OUT(1); /* 驱动时钟线为高电平，读取数据总线数值 */

    if(SOFT_IIC_DATA_READ == SOFT_IIC_NACK)
    {
        ret = SOFT_IIC_NACK; /*非应答信号*/
    }
    else
    {
        ret = SOFT_IIC_ACK; /*应答信号*/
    }

    SOFT_IIC_SCL_OUT(0); /* 驱动时钟线为低电平，准备下一个时钟周期 */
    SOFT_IIC_DELAY_US(i2c_delay_time); /* 延时 */

    return ret;
}


/* 发送数据 */
ErrorStatus SOFT_IIC_SendData(uint8_t data)
{
    /* 发送1个字节的数据 */
    for(uint8_t i = 0;i < 8;i++)
    {
        SOFT_IIC_SDA_OUT(data&(0x80 >> i)); /* 将数据的最高位先输出 */
        SOFT_IIC_DELAY_US(i2c_delay_time);  /* 延时 */
        SOFT_IIC_SCL_OUT(1); /* 当SCL拉高时SDA不能改变，否则将产生停止信号或开始信号，无法正确传输数据 */
        SOFT_IIC_DELAY_US(i2c_delay_time);  /* 延时 */
        SOFT_IIC_SCL_OUT(0); /* 当SCL拉低时SDA可以改变，否则将无法正确传输数据 */
    }

    /* 发送完之后等待ACK */
    if(SOFT_IIC_WaitAck() == SOFT_IIC_NACK)
    {
        SOFT_IIC_Stop(); /* 回应为NACK时表示无人回应，则先释放SDA总线 */
        return ERROR;
    }
    else
    {
        return SUCCESS;
    }
}

/* 发送地址和方向（读写），地址7位，方向1位 */
ErrorStatus SOFT_IIC_AddressMatching(uint8_t slave_addr,SOFT_IIC_Direction_TypeDef direction)
{
    /* 将地址和方向合并为一个字节，高7位为地址，低1位为方向 */
    uint8_t data = (slave_addr << 1) | direction; 
    return SOFT_IIC_SendData(data); /* 发送地址 */
}

/* 检验IIC总线上的从机设备 */
ErrorStatus SOFT_IIC_CheckDevice(uint8_t slave_addr)
{
    ErrorStatus temp = ERROR;

    /* 发送开始信号 */
    SOFT_IIC_Start(); 

    /* 呼叫从机，地址配对，若返回ACK则表示从机存在 */
    temp = SOFT_IIC_AddressMatching(slave_addr,SOFT_IIC_WRITE);
    if(temp != SUCCESS)
    {
        return temp;
    }
    /* 释放总线并发出停止信号 */
    SOFT_IIC_Stop();
    return SUCCESS;
}

/*****************************END OF FILE***************************************/

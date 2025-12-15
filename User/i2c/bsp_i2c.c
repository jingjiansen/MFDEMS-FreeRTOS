#include "i2c/bsp_i2c.h"
#include "dwt/bsp_dwt.h"  


/* 硬件IIC引脚配置 */
void IIC_PinConfig(void)
{
    GPIO_InitTypeDef gpio_initstruct = {0};
   
    RCC_APB2PeriphClockCmd(IIC_SCL_GPIO_CLK_PORT,ENABLE);
    IIC_SCL_OUT(1);
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_AF_OD;
    gpio_initstruct.GPIO_Pin    = IIC_SCL_GPIO_PIN;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SCL_GPIO_PORT,&gpio_initstruct); 
    
    RCC_APB2PeriphClockCmd(IIC_SDA_GPIO_CLK_PORT,ENABLE);
    IIC_SDA_OUT(1);
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_AF_OD;
    gpio_initstruct.GPIO_Pin    = IIC_SDA_GPIO_PIN;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SDA_GPIO_PORT,&gpio_initstruct);     
}


/* 硬件IIC模式配置 */
void IIC_Mode_Config(void)
{
    I2C_InitTypeDef i2c_initstruct = {0};    
    RCC_APB1PeriphClockCmd(IIC_I2CX_CLK_PORT,ENABLE);
    i2c_initstruct.I2C_Mode                 = I2C_Mode_I2C;
    /* 占空比：2 表示1个时钟周期内的高电平持续时间为66%，低电平持续时间为33%
       高电平用于数据保持，低电平用于数据切换 */
    i2c_initstruct.I2C_DutyCycle            = I2C_DutyCycle_2; /* 高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 */
    i2c_initstruct.I2C_OwnAddress1          = IIC_I2CX_OWN_ADDRESS7; /* 自身地址 */
    i2c_initstruct.I2C_Ack                  = I2C_Ack_Enable; /* 使能ACK */
    i2c_initstruct.I2C_AcknowledgedAddress  = I2C_AcknowledgedAddress_7bit; /* 7位地址模式 */      
    i2c_initstruct.I2C_ClockSpeed           = IIC_SPEED; /* 通信速率 */
    I2C_Init(IIC_I2CX, &i2c_initstruct);
}


/* 硬件IIC初始化 */
void IIC_Init(void)
{
    IIC_PinConfig();          /* 配置IIC GPIO引脚 */
    IIC_Mode_Config();        /* 模式配置：I2C模式，占空比2，7位地址，使能ACK，7位地址，通信速率 */
    I2C_Cmd(IIC_I2CX,ENABLE); /* 使能 HARD_IIC */
}

/*延时接口：用于维持高低电平*/
void IIC_DELAY_US(uint32_t time)
{
    DWT_DelayUs(time);
}


/* 发送开始信号 */
ErrorStatus IIC_Start(void)
{
    uint32_t check_times = CHECK_TIMES;
    
    /* 检测总线是否繁忙 */
    while(I2C_GetFlagStatus(IIC_I2CX,I2C_FLAG_BUSY))
    {
        check_times--;
        IIC_DELAY_US(1);
        if(check_times == 0)
        {
            return ERROR;
        }
    }
    
    /* 产生I2C 起始信号*/
    I2C_GenerateSTART(IIC_I2CX, ENABLE);
    check_times = CHECK_TIMES;

    /* 等待主模式选择完成：主模式：开始信号已经发送到总线、硬件确认总线控制权已获得、状态机准备后发送后续命令 */
    while(I2C_CheckEvent(IIC_I2CX,I2C_EVENT_MASTER_MODE_SELECT) == ERROR)
    {
        check_times--;
        IIC_DELAY_US(1);
        if(check_times == 0)
        {
            /* 发送停止信号方便下次通信使用*/
            IIC_Stop();
            return ERROR;
        }
    }
    return SUCCESS;
}


/* 发送停止信号 */
void IIC_Stop(void) 
{
    I2C_GenerateSTOP(IIC_I2CX,ENABLE);
}


/* 发送数据 */
ErrorStatus IIC_SendData(uint8_t data)
{
    uint32_t check_times = CHECK_TIMES;
    I2C_SendData(IIC_I2CX,data); /* 发送数据 */
    /* 等待数据发送完成 */   
    while(I2C_CheckEvent(IIC_I2CX,I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR)
    {
        check_times--;
        IIC_DELAY_US(1);
        if(check_times == 0)
        {
            IIC_Stop();
            return ERROR;
        }
    }
    return SUCCESS;
}


/* 发送地址和方向 */
ErrorStatus IIC_AddressMatching(uint8_t slave_addr,IIC_Direction_TypeDef direction)
{
    uint32_t check_times    = CHECK_TIMES;
    uint32_t event_temp     = I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED;
    
    /* 根据传输方向选择不同的事件标志 */
    if(direction == IIC_WRITE)
        event_temp = I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED; /* 传输 */
    else
        event_temp = I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED; /* 接收 */

    /* 发送地址 */
    I2C_Send7bitAddress(IIC_I2CX,(slave_addr<<1)|direction,I2C_Direction_Transmitter);  //本质与I2C_SendData(IIC_I2CX, (slave_addr<<1)|读写操作位)一样

    /* 等待地址发送完成 */
    while(I2C_CheckEvent(IIC_I2CX,event_temp) == ERROR)
    {
        check_times--;
        IIC_DELAY_US(1);
        if(check_times == 0)
        {
            IIC_Stop();
            return ERROR;
        }
    }
    return SUCCESS;
}


/* 检查总线上的从机设备 */
ErrorStatus IIC_CheckDevice(uint8_t slave_addr)
{
    ErrorStatus temp = ERROR;
    
    /* 发出开始信号并检测总线是否繁忙 */
    temp = IIC_Start();
    if(temp != SUCCESS)
    {
        return temp;
    }
   
    /* 呼叫从机,地址配对，若返回ACK表示从机存在*/
    temp = IIC_AddressMatching(slave_addr,IIC_WRITE);
    if(temp != SUCCESS)
    {
        return temp;
    }
    
    /* 释放总线并发出停止信号 */
    IIC_Stop();
    return SUCCESS;  
}

/*********************************************END OF FILE**********************/

/**
  ******************************************************************************
  * @file       bsp_dht11.c
  * @author     embedfire
  * @version     V1.0
  * @date        2024
  * @brief      温湿度传感器应用函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台  ：野火 STM32F103C8T6-STM32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
  
#include "dht11/bsp_dht11.h" 
#include "dwt/bsp_dwt.h"  

/**
  * @brief  初始化控制 DHT11 的IO
  * @param  无
  * @retval 无
  */
void DHT11_GPIO_Config(void)
{
    /* 定义一个 GPIO 结构体 */
    GPIO_InitTypeDef gpio_initstruct = {0};
      
    /* 开启 DHT11 相关的GPIO外设/端口时钟 */
    RCC_APB2PeriphClockCmd(DHT11_DATA_GPIO_CLK_PORT,ENABLE);
    
    /*选择要控制的GPIO引脚、设置GPIO模式为 上拉输入、设置GPIO速率为50MHz*/
    gpio_initstruct.GPIO_Pin    = DHT11_DATA_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = GPIO_Mode_IPD;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_DATA_GPIO_PORT,&gpio_initstruct);
   
}


/**
 * @brief  DHT11_DATA 引脚模式配置
 * @param  mode:引脚模式
 * @retval 无
 */
void DHT11_DataPinModeConfig(GPIOMode_TypeDef mode)
{
    
    /* 定义一个 GPIO 结构体 */
    GPIO_InitTypeDef gpio_initstruct = {0};

    /*选择要控制的GPIO引脚、设置GPIO模式为 mode、设置GPIO速率为50MHz*/
    gpio_initstruct.GPIO_Pin    = DHT11_DATA_GPIO_PIN;
    gpio_initstruct.GPIO_Mode   = mode;
    gpio_initstruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_DATA_GPIO_PORT,&gpio_initstruct);
    
}

/**
 * @brief  DHT11_DATA 引脚模式配置
 * @param  无
 * @retval dht11_readbyte_temp:返回数据（8bit）
 */
uint8_t DHT11_ReadByte(void)
{
    uint8_t dht11_readbyte_temp = 0;
    
    for(uint8_t i = 0;i<8;i++)
    {
        /* 延时x us 这个延时需要大于低电平开始标志持续的时间即可 */
//        DWT_Delay_Us(54); 
        while(DHT11_DATA_IN() == Bit_RESET);//每bit以54us低电平标置开始，轮询直到从机发出的54us低电平结束 
        
        /* 延时x us 这个延时需要大于数据0持续的时间,小于数据0+低电平开始信号持续时间即可 */
        DWT_DelayUs(40);   // 27 ~ 74 (27+54) //DHT11 以23~27us的高电平表示“0”，以68-74us高电平表示“1”  
        
        if(DHT11_DATA_IN() == Bit_SET) // x us后仍为高电平表示数据“1” 
        {
            /* 等待数据1的高电平结束 */
            while(DHT11_DATA_IN() == Bit_SET);
            dht11_readbyte_temp |=(uint8_t)(0x1<<(7-i)); //把位7-i位置1，MSB先行 
        }
        else  // x us后为低电平表示数据“0” 
        {
            dht11_readbyte_temp &=(uint8_t)(~(0x1<<(7-i))) ; //把位7-i位清0，MSB先行 
        }
    }
    
    return dht11_readbyte_temp;

}

/**
 * @brief  一次完整的数据传输为40bit，高位先出,
 * @param  dht11_data:数据接收区
 * @note   8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和(每次读出的温湿度数值是上一次测量的结果，欲获取实时数据,需连续读取2次，但不建议连续多次读取传感器，每次读取传感器间隔大于2秒即可获得准确的数据。)
 * @retval ERROR：失败，SUCCESS：成功
 */
ErrorStatus DHT11_ReadData(DHT11_DATA_TYPEDEF *dht11_data)
{
    uint8_t count_timer_temp = 0;
    
    /*输出模式*/
    DHT11_DataPinModeConfig(GPIO_Mode_Out_OD);//1、配置开漏输出,前提有上拉电阻，且跳变速度符合应用，高电平驱动能力由上拉电阻决定，电阻大小和反应速度成反比，和功耗成正比
        
    /*起始信号*/
    DHT11_DATA_OUT(0);//总线空闲状态为高电平,主机把数据总线（SDA）拉低等待DHT11响应,主机把总线拉低一段时间至少18ms（最大不得超过30ms）,保证DHT11能检测到起始信号。
    DWT_DelayMs(20);
    
    DHT11_DATA_OUT(1);//关闭NMOS
    
    /*主机设为输入判断从机响应信号*/
    DHT11_DataPinModeConfig(GPIO_Mode_IPU);//1、配置浮空输入模式, 总线由外部上拉电阻拉高,表示空闲的电平; 2、配置上拉输入模式, 总线由上拉电阻拉高，表示空闲的电平。

    DWT_DelayUs(20);//等待上拉生效且回应,主机发送开始信号结束后,延时等待20-40us后
    
    /*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/  
    if(DHT11_DATA_IN()== Bit_RESET) 
    {
        count_timer_temp = 0;
        /*轮询直到从机发出 的83us 低电平 应答信号结束*/ 
        while(DHT11_DATA_IN() == Bit_RESET)
        {
            if(count_timer_temp++ >83)//超时计数,传感器等待外部信号低电平结束，延迟后DHT11的DATA引脚处于输出状态，输出83微秒的低电平作为应答信号
            {
                return ERROR;
            }  
            DWT_DelayUs(1);
        }
        
        count_timer_temp = 0;
        /*轮询直到从机发出 的87us 高电平 通知主机接收数据*/  
        while(DHT11_DATA_IN() == Bit_SET)
        {
            if(count_timer_temp++ >87)//超时计数,传感器等待外部信号低电平结束，延迟后DHT11的DATA引脚处于输出状态，输出87微秒的高电平作为应答信号
            {
                return ERROR;
            }  
            DWT_DelayUs(1);
        }
        
        /* 开始接收数据 */
        dht11_data->humi_int  = DHT11_ReadByte();        //湿度高8位
        dht11_data->humi_deci = DHT11_ReadByte();        //湿度低8位
        dht11_data->temp_int  = DHT11_ReadByte();        //温度高8位
        dht11_data->temp_deci = DHT11_ReadByte();        //温度低8位
        dht11_data->check_sum = DHT11_ReadByte();        //校验和
        
        /* 读取结束 */
        DWT_DelayUs(54);
        
        /*检查读取的数据是否正确*/
        
        if(dht11_data->check_sum == dht11_data->humi_int+dht11_data->humi_deci+dht11_data->temp_int+dht11_data->temp_deci)
        {
            return SUCCESS;
        }
        else
        {
            return ERROR;
        }
    }
    else
    {
        return ERROR;
    }
    
    
}
/*********************************************END OF FILE**********************/

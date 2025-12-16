## 迁移FreeRTOS

本项目是《裸机版裸机版多功能桌面环境监测系统》在FreeRTOS上的实现，旨在巩固学习到的FreeRTOS源码的相关知识。本项目是学习野火《FreeRTOS内核实现与应用开发实战指南》课程的学习成果[视频源址](https://www.bilibili.com/video/BV1Jx411X7NS?spm_id_from=333.788.videopod.episodes&vd_source=eaf8b4b66b7bc2a0cae5103b944514eb)。文中图片和项目部分资料均来源于该视频。

### 迁移

1. 复制《裸机版多功能桌面环境监测系统》工程；
2. 将FreeRTOS源文件添加进项目工程；
3. 将FreeRTOS头文件添加进项目工程；
4. 修改配置文件并将其添加进项目；
5. 注释掉`stm32f10x_it.c`文件中的`SVC_Handler`和`PendSV_Handler`中断服务程序，FreeRTOS中已经定义。

### 验证

实现两个任务：`KEY_Task`和`LED_Task`，前者接收消息队列中的消息，从而判断是哪个按键按下，并开相应的灯（按键1红灯、按键2蓝灯）作为标志；后者则一直翻转绿灯。

1. 修改`stm32f10x_it.c`文件中的系统时钟回调函数`SysTick_Handler`，将FreeRTOS实现的接口引入其中。SysTick作为FreeRTOS的系统心跳，是整个操作系统的时基。
2. 修改`stm32f10x_it.c`文件中的按键1、按键2的中断回调函数。
3. 修改`main()`函数：创建任务、创建消息队列。

运行程序，如果配置正常应当看到绿灯闪烁，按下按键1红灯亮，按下按键2蓝灯亮，再次按下则灭，且串口有相关信息打印出来。

## 重构裸机版本

### 多任务环境下DHT11数据读取问题

#### 问题分析

1. 之前进行DHT11数据读取的时机为任务延时（500ms）达到且当前处于温湿度菜单下，由于数据是显示时才通过单总线协议读取的，而非在显示时已经读取好，因此存在第一次卡顿现象；
2. 显示任务中关于DHT11数据显示的部分分为两步：第一次进入则显示文字部分：“温度：、湿度：”，等到第二次刷新时才会加载DHT11数据并进行显示。这样会造成显示上的不连贯，导致卡顿。此外，在多任务下，如果前后两次刷新之间（即两次任务调度之间）被延时（可能由于外部中断），则会进一步加剧卡顿。但这种更新方式为局部刷新，只刷新变化的数据区域，从而有利于提升数据传输效率。
3. 由于显示任务和读取任务共享温湿度数据`dht11_data`，但目前并未做任何的线程安全操作，可能导致数据不一致和数据竞争的风险。
4. DHT11任务中使用DWT来控制通信时序；在字节读取函数`DHT11_ReadByte()`中通过`while`循环阻塞低电平，直到高电平出现，如果出现DHT11硬件故障，任务将永远阻塞。

#### 解决方法

1. 针对问题1：由于目前温湿度读取在专门任务循环中进行，无需设置基于系统时钟SysTick中断的刷新机制，采用FreeRTOS自带的任务延时`vTaskDelay(500)`来控制读取频率。
2. 针对问题2：关于两次刷新，优化为一次刷新，即在一次任务循环中进行判断，如果是第一次进入则需要刷新整个屏幕，包括数据获取。第二次刷新时则仅刷新数据部分。从而保持了局部刷新并避免了首次卡顿。

    1. 在`app_oled.c`的任务循环中修改和温湿度相关的代码逻辑。

        ```C
                            /* 温湿度 */
                            case 0x20:
                                if(menu == 0x21 || menu == 0x22) 
                                {
                                    /* 第一次进入需要刷新整个屏幕 */
                                    if(menu == 0x21) 
                                    {
                                        OLED_CLS();
                                        OLED_ShowChinese_F16X16(1,1,6);
                                        OLED_ShowChinese_F16X16(1,2,8);
                                        OLED_ShowChinese_F16X16(1,3,13);
                                    
                                        OLED_ShowChinese_F16X16(3,1,7);
                                        OLED_ShowChinese_F16X16(3,2,8);
                                        OLED_ShowChinese_F16X16(3,3,13);

                                        menu =0x22;
                                    }

                                    /* 非阻塞接收DHT11数据（10ms超时） */
                                    if(xQueueReceive(xDht11Queue, &display_data, pdMS_TO_TICKS(10)) == pdTRUE) 
                                    {
                                        printf("DHT11数据队列接收成功!\r\n");

                                        if(display_data.temp_deci&0x80) 
                                        {
                                            sprintf(str_temp,"-%d.%d",display_data.temp_int,display_data.temp_deci);
                                        }
                                        else 
                                        {
                                            sprintf(str_temp,"%d.%d",display_data.temp_int,display_data.temp_deci);
                                        }
                                        if(display_data.humi_deci&0x80) 
                                        {
                                            sprintf(str_humi,"-%d.%d %%RH",display_data.humi_int,display_data.humi_deci);
                                        }
                                        else 
                                        {
                                            sprintf(str_humi,"%d.%d %%RH",display_data.humi_int,display_data.humi_deci);
                                        }
                                    }
                                    else 
                                    {
                                        /* 使用旧值时，即display_data的值不变*/
                                        printf("DHT11数据队列接收失败，使用旧值!\r\n");
                                    }

                                    OLED_ShowChinese_F16X16(1,7,14); /* 显示温度单位 */
                                    OLED_ShowString_F8X16(1,8,(uint8_t*)str_temp);
                                    OLED_ShowString_F8X16(3,8,(uint8_t*)str_humi);
                                }
                                break;
        ```
3. 针对问题3：使用消息队列来作为DHT11任务和显示任务Display数据传输的桥梁。由于队列为空时显示任务会被挂起，从而造成卡顿，因此设置超时时间为10ms来避免长时挂起。进一步可以取消掉全局变量`dht11_data`，因为数据转发已经由队列实现。

    1. 在`app_dht11.h`中添加消息队列相关变量和头文件。

        ```C
        #include "FreeRTOS.h"
        #include "queue.h"
        extern QueueHandle_t xDht11Queue;
        void DHT11_Init(void);
        ```
    2. 在`app_dht11.c`的DHT11任务函数中增加消息入队操作。

        ```C
        /* DHT11 初始化*/
        void DHT11_Init(void) {
            /* 创建数据队列，深度为1，使用覆盖模式 */
            xDht11Queue = xQueueCreate(1,sizeof(DHT11_DATA_TYPEDEF));
            if(xDht11Queue == NULL) {
                printf("dht11 queue create error!\r\n");
            }
            else {
                printf("dht11 queue create success!\r\n");
            }
        }

        void DHT11_Task(void *pvParameters) {
            DHT11_DATA_TYPEDEF last_valid_data = {0};
            printf("dht11 task start!\r\n");
            while(1) {
                if(DHT11_ReadData(&last_valid_data) == SUCCESS) {
                    /* 发送数据到队列 */
                    if(xQueueOverwrite(xDht11Queue, &last_valid_data) == pdTRUE) {
        			......
        }
        ```
4. 针对问题4：添加超时保护的阻塞等待。至于在多任务下是否会发送因任务切换而导致数据读取错误的问题，这里并不使用临界区进行保护，而是通过数据是否有效和校验和是否正确来判断，即如果发生上述情况而导致数据错误，则最终会体现在数据无效或校验和不对的情况上，此时直接返回`ERROR`，进入下次读取。不使用临界区进行保护的原因在于保护区域要么耗时太长（连接建立阶段），要么多次调用（字节读取函阶段），都会导致长时间关中断、且临界区过大不符合临界区使用原则。

    1. `DHT11_ReadData()`函数

        ```C
        /**
         * @brief  一次完整的数据传输为40bit，高位先出,
         * @param  dht11_data:数据接收区
         * @note   8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和
         * @note   通过读取数据总线（SDA）的电平，判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行
         * @retval ERROR：失败，SUCCESS：成功
         */
        ErrorStatus DHT11_ReadData(DHT11_DATA_TYPEDEF *dht11_data) {

            uint8_t count_timer_temp = 0;
            
            /* 步骤1：主机设置输出状态，拉低数据线18ms以上，然后设置为输入模式，释放数据线 */
            DHT11_DataPinModeConfig(GPIO_Mode_Out_OD);  /* 配置为开漏输出模式，由外部上拉电阻拉高电平 */
            DHT11_DATA_OUT(0);                          /* 拉低数据总线（SDA）*/
            DWT_DelayMs(20);                            /* 阻塞等待20ms（确保DHT11能检测到起始信号）*/
            DHT11_DATA_OUT(1);                          /* 释放数据总线（SDA）*/
            DHT11_DataPinModeConfig(GPIO_Mode_IPU);     /* 配置为上拉输入模式，由外部上拉电阻拉高电平 */
            DWT_DelayUs(20);                            /* 等待上拉生效且回应,主机发送开始信号结束后,延时等待20-40us后 */
            
            /* 步骤2：从机接收到起始信号后，先拉低数据线83us以上，然后拉高数据线87us以上 */
            /* 通过读取数据总线（SDA）的电平，判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/
            if(DHT11_DATA_IN()== Bit_RESET) {
                /* 低电平信号 */
                count_timer_temp = 0;
                /* 轮询检测DHT11发出的低电平信号的持续时间，由于之前等待上拉延时了20us，因此这里计数值应该不到83就会直接跳出 */
                while(DHT11_DATA_IN() == Bit_RESET) {
                    if(count_timer_temp++ > 83) {
                        return ERROR;
                    }  
                    DWT_DelayUs(1);
                }
                        
                /* 高电平信号 */
                count_timer_temp = 0;
                /* 轮询检测DHT11发出的高电平信号的持续时间（87us） */
                while(DHT11_DATA_IN() == Bit_SET) {
                    if(count_timer_temp++ > 87) {
                        return ERROR;
                    }  
                    DWT_DelayUs(1);
                }
                
                /* 应答信号接收成功并且已建立连接，数据接收：连续读取40字节 */
                dht11_data->humi_int  = DHT11_ReadByte();        /* 湿度高8位 */
                dht11_data->humi_deci = DHT11_ReadByte();        /* 湿度低8位 */
                dht11_data->temp_int  = DHT11_ReadByte();        /* 温度高8位 */
                dht11_data->temp_deci = DHT11_ReadByte();        /* 温度低8位 */
                dht11_data->check_sum = DHT11_ReadByte();        /* 校验和 */

                /* 最后一位数据发送后，从机将拉低数据总线SDA54us，随后从机释放总线 */
                DWT_DelayUs(54);
                
                /* 数据有效性 */
                if(dht11_data->humi_int == 0xFF || dht11_data->humi_deci == 0xFF || dht11_data->temp_int == 0xFF || dht11_data->temp_deci == 0xFF || dht11_data->check_sum == 0xFF) {
                    return ERROR;
                }
                
                /* 校验和：加和前4个字节数据，校验和字节为加和结果的低8位 */
                if(dht11_data->check_sum == dht11_data->humi_int+dht11_data->humi_deci+dht11_data->temp_int+dht11_data->temp_deci) {            
                    return SUCCESS;
                }
                else {
                    return ERROR;
                }
            }
            else {
                return ERROR;
            }
        }
        ```
    2. `DHT11_ReadByte()`函数

        ```C
        /* 读取1个字节，即8bit */
        uint8_t DHT11_ReadByte(void) {
            uint8_t i, temp = 0;
            uint32_t timeout_counter;

            for(i = 0;i<8;i++) {
                /* 从机发送数据：每位数据以54us低电平标置开始，高电平持续时间为23~27us表示“1”，高电平持续时间为68~74us表示“0” */
                timeout_counter = 0;
                
                /* 起始54us低电平 */
                while(DHT11_DATA_IN() == Bit_RESET && timeout_counter < 54) {
                    timeout_counter++;
                    DWT_DelayUs(1);
                }
                if(timeout_counter >= 54) { /* 低电平时间超过54us，则返回错误值 */
                    return 0xFF;
                }
                
                /* 高电平出现之后延时40us，如果仍为高电平表示数据“1”，如果为低电平表示数据“0” */
                /* 理解：如果是“1”，则40us之后仍旧是高电平，如果是“0”，则40us之后为低电平（进入下一位的接收） */
                DWT_DelayUs(40);

                /* 高电平：“1” */
                if(DHT11_DATA_IN() == Bit_SET) {
                    temp |=(uint8_t)(0x1<<(7-i)); /* 把位7-i位置1，MSB先行（字节高位先发送） */
                }
                /* 低电平：“0” */
                else {
                    temp &=(uint8_t)(~(0x1<<(7-i))) ; /* 把位7-i位清0，MSB先行 */
                }

                timeout_counter = 0;
                /* 54us低电平后：高电平持续时间为23~27us表示“1”，高电平持续时间为68~74us表示“0”
                   这里过滤掉高电平时间，如果在延时30us之后高电平还未转为低电平，即进入下一位数据发送，则返回错误值 */
                while(DHT11_DATA_IN() == Bit_SET && timeout_counter < 35) {
                    timeout_counter++;
                    DWT_DelayUs(1);
                }
                if(timeout_counter >= 35) { /* 高电平时间超过27us，则返回错误值 */
                    return 0xFF;
                }
            }
            return temp;
        }
        ```

### 多任务环境下IIC资源竞争问题

#### 问题分析

在多任务环境下，如果多个任务同时访问IIC总线（虽然目前项目仅有显示任务访问），会导致IIC总线状态混乱、数据传输错误、硬件IIC控制器状态机异常的问题。

#### 解决方法

在驱动层`bsp_i2c.h、bsp_02c.c`中对IIC操作加互斥锁实现线程安全，OLED驱动层调用线程安全的IIC驱动函数即可。

1. 在`bsp_i2c.h`中声明互斥信号量`xIICSemaphore`、在`IIC_Init()`中初始化信号量、增加加锁和解锁函数。

    ```C
    // bsp_i2c.h
    /* IIC获取锁默认超时时间 */
    #define IIC_DEFAULT_TIMEOUT_MS  10
    /* IIC互斥信号量声明 */
    extern SemaphoreHandle_t xIICSemaphore;


    // bsp_i2c.c
    /* IIC互斥信号量定义 */
    SemaphoreHandle_t xIICSemaphore = NULL;


    /* 硬件IIC初始化 */
    void IIC_Init(void)
    {
        IIC_PinConfig();          /* 配置IIC GPIO引脚 */
        IIC_Mode_Config();        /* 模式配置：I2C模式，占空比2，7位地址，使能ACK，7位地址，通信速率 */
        I2C_Cmd(IIC_I2CX,ENABLE); /* 使能 HARD_IIC */

        /* 创建IIC互斥信号量 */
        if(xIICSemaphore == NULL)
        {
            xIICSemaphore = xSemaphoreCreateMutex();
            if(xIICSemaphore == NULL)
            {
                printf("IIC semaphore create error!\r\n");
            }
        }
    }

    /* 获取IIC锁 */
    ErrorStatus IIC_Lock(uint32_t timeout_ms)
    {
        if(xIICSemaphore == NULL) return ERROR;
        BaseType_t xSemaphoreTakeStatus = xSemaphoreTake(xIICSemaphore, pdMS_TO_TICKS(timeout_ms));
        return (xSemaphoreTakeStatus == pdTRUE) ? SUCCESS : ERROR;
    }

    /* 释放IIC锁 */
    void IIC_Unlock(void)
    {
        if(xIICSemaphore != NULL)
        {
            xSemaphoreGive(xIICSemaphore);
        }
    }

    ```
2. 在发送开始信号函数`IIC_Start()`中加锁，在发送停止信号函数`IIC_Stop()`中解锁即可实现线程同步。具体地，在`IIC_CheckDevice()`函数中通过成对调用`IIC_Start()`和`IIC_Stop()`来实现正确的解锁，且在`IIC_AddressMatching()`中处理了发生错误时通过调用`IIC_Stop()`来解除总线占用和解锁，`IIC_SendData()`函数同样如此。

    ```C
    /* 发送开始信号 */
    ErrorStatus IIC_Start(void)
    {    
        /* 加锁 */
        if(IIC_Lock(IIC_DEFAULT_TIMEOUT_MS) == SUCCESS)
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
            /* 这里不立即解锁是因为后续操作可能需要继续使用IIC总线 */
        }
    }

    /* 发送停止信号 */
    void IIC_Stop(void) 
    {
        I2C_GenerateSTOP(IIC_I2CX,ENABLE);

        /* 停止信号后解锁IIC总线 */
        IIC_Unlock();
    }
    ```

### 按键3修改为中断检查而非轮询

#### 问题分析及解决方法

按键3使用PB15引脚，其和按键2，PC13引脚共用一个中断向量号`EXTI15_10_IRQn`，因此两者共用一个中断回调函数`EXTI15_10_IRQHandler`。使用`KEY23_EXTI_IRQHANDLER`作为两者的中断回调函数别名。

```C
void KEY23_EXTI_IRQHANDLER(void)
{
    uint32_t ulReturn;
    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    if(EXTI_GetITStatus(KEY2_EXTI_LINE) == SET)
    {
        right_shift_flag = 1; /* 标记右移 */
        menu_show_flag = 1;  /* 标记显示菜单 */
        EXTI_ClearITPendingBit(KEY2_EXTI_LINE);
    }

    if(EXTI_GetITStatus(KEY3_EXTI_LINE) == SET)
    {
        enter_flag = 1;     /* 标记确认 */
        menu_show_flag = 1;  /* 标记显示菜单 */
        EXTI_ClearITPendingBit(KEY3_EXTI_LINE);
    }

    taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}
```

### 显示逻辑优化以及全局变量的线程安全优化

1. 显示任务中：`menu_show_flag、left_shift_flag、right_shift_flag、enter_flag`四个全局变量在按键中断回调函数中被标记，在显示任务`Display_Task`中被使用，以驱动显示内容变化。而`menu、led_state_flag、wooden_fish_num`三个变量并未在其他任务或中断中使用，因此将其作为任务内的局部变量。
2. 显示逻辑部分，由于按键3修改为中断方式，且DHT11任务不再使用`content_show_flag`标记，因此修改了显示逻辑，并通过增加一个局部变量`add_flag`来控制打坐界面木鱼状态的切换。

### 多任务下的USART优化

#### 问题分析

USART_COM当前为轮询发送（等待TXE和TC），接收配置为中断（RX），并且项目中任务直接调用`printf`（非线程安全，占栈大，会阻塞）.在多任务下如果使用消息队列的方式通过串口发送调试信息将是线程安全的，但FreeRTOS系统启动前无法调用队列，即在进行硬件初始化、内核初始化、任务创建以及调度器启动这些阶段内无法使用队列。

#### 解决方法

##### 方法

分为两阶段：调度器启动前和调度器启动后。

- 调度器启动前（`SYS_PHASE_EARLY`）：轮询发送。
- 调度器启动后（`SYS_PHASE_RTOS_RUNNING`）：切换到中断+DMA模式。发送时使用DMA进行后台搬用，并且使用互斥信号量实现同步；接收时使用DMA循环填充线性缓冲区，并使用空闲帧中断和队列来判断接收完成和数据缓存。
- 发送流控制：

  - 互斥量保证多任务同时调用时顺序排队；
  - 复制到内部`TxDMABuffer`避免用户数据被后续任务篡改；
  - 二值信号量完成“DMA完成”同步，支持有限超时；
- 接收流控制：

  - DMA循环模式持续将串口数据搬进`RxBuffer`；
  - 可选择“空闲中断”或“半\/全传输中断”触发回调，再把已收数据通过队列抛给任务；
  - 上层`UART_ReceiveByte()`统一为阻塞式队列读取；
- 统一打印函数：

  - `uart_printf()`在内部自动识别阶段，调度器启动前使用轮询发送，启动后使用DMA发送。
- 整体使用宏开关控制。

##### 用到的数据结构

- 互斥锁：

  - 用于确保同一时间只有一个任务可以使用串口发送功能，确保消息按顺序发送，不会交叉。保护的关键资源包括：DMA发送缓冲区、发送状态标志、DMA通道配置寄存器。
  - `DebugUart.TxMutex = xSemaphoreCreateMutex();`

- 二进制信号量：

  - 用于DMA传输完成通知，DMA传输完成之后释放信号量，发送任务可以阻塞等待DMA传输完成。
  - `DebugUart.TxCompleteSem = xSemaphoreCreateBinary();`
- 接收队列：

  - 用于同步相关的接收任务。大小`QUEUESIZE=64`字节。
  - `DebugUart.RxQueue = xQueueCreate(QUEUESIZE, sizeof(uint8_t));`
- 接收缓冲区：

  - 用于缓存DMA接收数据。大小`RXDMABUFFER_SIZE=1024`字节。
  - `DebugUart.RxBuffer = pvPortMalloc(DebugUart.RxBufferSize);`
- 发送缓冲区：

  - 用于缓存DMA发送数据。大小`TXDMABUFFER_SIZE=256`字节。
  - `DebugUart.TxDMABuffer = pvPortMalloc(DebugUart.TxDMABufferSize);`

## 版本修订

| 序号 | 修订时间   | 修订概要 |
| ------ | ------------ | ---------- |
| 1    | 2025-12-12 | 首次提交 |
| 2    | 2025-12-16 | 提交内容 |

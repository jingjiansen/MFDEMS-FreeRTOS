# 项目2：FreeRTOS版多功能桌面环境监测系统

## 迁移FreeRTOS

### 迁移

1. 复制《裸机版多功能桌面环境监测系统》工程；
2. 将FreeRTOS源文件添加进项目工程；
3. 将FreeRTOS头文件添加进项目工程；
4. 修改配置文件并将其添加进项目；
5. 注释掉`stm32f10x_it.c`​文件中的`SVC_Handler`​和`PendSV_Handler`中断服务程序，FreeRTOS中已经定义。

### 验证

实现两个任务：`KEY_Task`​和`LED_Task`，前者接收消息队列中的消息，从而判断是哪个按键按下，并开相应的灯（按键1红灯、按键2蓝灯）作为标志；后者则一直翻转绿灯。

1. 修改`stm32f10x_it.c`​文件中的系统时钟回调函数`SysTick_Handler`，将FreeRTOS实现的接口引入其中。SysTick作为FreeRTOS的系统心跳，是整个操作系统的时基。
2. 修改`stm32f10x_it.c`文件中的按键1、按键2的中断回调函数。
3. 修改`main()`函数：创建任务、创建消息队列。

运行程序，如果配置正常应当看到绿灯闪烁，按下按键1红灯亮，按下按键2蓝灯亮，再次按下则灭，且串口有相关信息打印出来。

## 重构

|序号|中断|中断优先级|GPIO引脚|外部中断线|中断回调函数||
| :----: | :---------: | :----------: | :-------------------: | :----------: | :------------: | :-: |
|1|按键1中断|7|PA_0|EXTI_0|​`KEY1_EXTI_IRQHANDLER`||
|2|按键2中断|7|PC_13|EXTI_13|​`KEY2_EXTI_IRQHANDLER`||
|3|串口|9|PA_9(TX)、PA_10(RX)||​`DEBUG_IRQHANDLER`||
|4|||||||
|5|||||||
|6|||||||

### DHT11数据读取在多任务下的问题

1. 之前进行DHT11数据读取的时机为任务延时达到且当前处于温湿度菜单下，由于数据是显示时才通过单总线协议读取的，而非在显示时已经读取好，因此存在第一次卡顿现象
2. 显示任务中关于DHT11数据显示的部分分为两步：第一次进入则显示文字部分：“温度：；湿度：”，等到第二次刷新时才会加载DHT11数据并进行显示。这样会造成显示上的不连贯，导致卡顿。此外，在多任务下，如果前后两次刷新之间（即两次任务调度之间）被延时（可能由于外部中断），则会进一步加剧卡顿。但这种更新方式为局部刷新，只刷新变化区域，这样有利于提升数据传输效率，因为变化的只是数据部分，没必要每次都整屏刷新。
3. 由于显示任务和读取任务共享温湿度数据`dht11_data`，但目前并未做任何的线程安全操作，可能导致数据不一致和数据竞争的风险。
4. DHT11任务中使用DWT来控制通信时序；在字节读取函数`DHT11_ReadByte()`​中通过`while`循环阻塞低电平，直到高电平出现，如果出现DHT11硬件故障，任务将永远阻塞。

### 解决方法

1. 针对问题1：由于目前温湿度读取在专门任务循环中进行，无需设置基于系统时钟SysTick中断的刷新机制。
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
4. 针对问题4：添加超时保护的阻塞等待。至于在多任务下是否会发送因任务切换而导致数据读取错误的问题，这里并不使用临界区进行保护，而是通过判断数据是否有效和校验和是否正确来判断，即如果发生上述情况而导致数据错误，则最终会体现在数据无效或校验和不对的情况下，此时直接返回`ERROR`，进入下次读取。不使用临界区进行保护的原因在于保护区域要么耗时太长（连接建立阶段），要么多次调用（字节读取函阶段），都会导致长时间关中断、临界区过大，不符合临界区使用原则。

    1. ​`DHT11_ReadData()`函数

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
    2. ​`DHT11_ReadByte()`函数

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

### 按键3修改为中断检查而非轮询

按键3使用PB15引脚，其和按键2，PC13引脚共用一个中断向量号`EXTI15_10_IRQn`​，因此两者共用一个中断回调函数`EXTI15_10_IRQHandler`​。使用`KEY23_EXTI_IRQHANDLER`作为两者的中断回调函数别名。

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

显示任务中：`menu_show_flag、left_shift_flag、right_shift_flag、enter_flag`​四个全局变量在按键中断回调函数中被标记，在显示任务`Display_Task`中被使用，以驱动显示内容变化。在多任务环境下存在数据一致性的问题，例如当任务依据某一全局变量当前值而进入了相应条件分支，此时中断发生改变了该全局变量的值，即此时条件已不成立，但相应分支仍旧会执行。这里使用事件组来实现按键标志的线程安全管理。

### USART轮询发送与多任务共用（非线程安全）

1. 在多任务下如果使用消息队列的方式将是线程安全的，但在FreeRTOS系统启动前无法调用队列，即在进行硬件初始化、内核初始化、任务创建以及调度器启动这些阶段内无法使用队列。因此分阶段设计，系统启动前使用简单的全局变量和标志，且此时无多任务，执行顺序线性，因此也不会出现冲突；系统启动后使用消息队列。
2. USART_COM当前为轮询发送（等待TXE和TC），接收配置为中断（RX），并且项目中任务直接调用`printf`（非线程安全，占栈大，会阻塞）；用队列或stream buffer做事务化排队，由UART或IRQ+DMA复杂发送，避免多个任务直接轮询发送，不要在多个任务直接调用printf，任务用snprintf格式化到本地缓冲，然后xQueueSend缓冲指针或拷贝到UART TX队列，UART任务再发送（启动DMA传输并等待完成中断）；适合大块日志，高吞吐

‍

‍

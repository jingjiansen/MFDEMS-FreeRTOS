#include "oled/app_oled.h" 
#include "oled/bsp_i2c_oled.h"
#include "fonts/bsp_fonts.h"
#include <stdio.h>

/* 板级初始化 */
#include "dwt/bsp_dwt.h" 
#include "key/bsp_gpio_key.h"
#include "led/bsp_gpio_led.h"
#include "i2c/bsp_i2c.h"
#include "debug/bsp_debug.h"
#include "dht11/bsp_dht11.h"


/* 应用初始化 */
#include "dht11/app_dht11.h"

#include "FreeRTOS.h"
#include "task.h"

uint8_t menu_show_flag = 0;
uint8_t left_shift_flag = 0;
uint8_t right_shift_flag = 0;
uint8_t enter_flag = 0;



/* @brief  板级初始化
 * @param  无
 * @retval 无
 */
void Bsp_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    DWT_Init();
    KEY_Init();
    LED_GPIO_Config();
    IIC_Init();
    DEBUG_USART_Init();
    DHT11_GPIO_Config();
}


/* @brief  应用初始化
 * @param  无
 * @retval 无
 */
void App_Init(void)
{
    DHT11_Init();
    OLED_Init();
    OLED_CLS();

    OLED_DrawBitMap((128-113)/2, 0, 113, 64, (uint8_t*)dog4);
    DWT_DelayMs(200);
    OLED_DrawBitMap((128-113)/2, 0, 113, 64, (uint8_t*)dog3);
    DWT_DelayMs(300);
    OLED_DrawBitMap((128-113)/2, 0, 113, 64, (uint8_t*)dog1);
    DWT_DelayMs(500);

    menu_show_flag = 1;
}



/**
 * @brief  OLED任务
 * @param  无
 * @retval 无
 */
void Display_Task(void)
{
    uint8_t menu = 0;
    uint8_t led_state_flag = 0;
    uint32_t wooden_fish_num = 0;

    menu = 0;
    led_state_flag = 0;
    wooden_fish_num = 0;

    /* 缓存DHT11队列数据 */
    DHT11_DATA_TYPEDEF display_data = {0};

    /* 缓存温湿度显示字符串 */
    char str_temp[512] = {NULL};
    char str_humi[512] = {NULL};
    sprintf(str_temp, "%d.%d", display_data.temp_int, display_data.temp_deci);
    sprintf(str_humi, "%d.%d %%RH", display_data.humi_int, display_data.humi_deci);

    uint8_t add_flag = 0;
    char str_temp1[128] = {NULL};


    while(1)
    {
        /* 按键3按下判断是否切换到内容界面 */
        if(enter_flag == 1)
        {
            /* 菜单选择：低4位为0时，说明当前处于1级菜单；接下来的动作是：一级菜单进入二级菜单显示内容*/
            if((menu&0x0f)==0)
            {
                OLED_CLS();             /* 清屏 */

                /* 菜单1进入还是菜单1 */
                if(menu == 0x00)
                {
                    menu = menu;
                }
                /* 菜单3进入则显示灯具内容 */
                else if(menu == 0x30)
                {
                    /* 灯具内容：开关灯，依据led_state_flag判断当前状态切换显示的位图*/
                    if(led_state_flag == 1)
                    {
                        menu = (menu&0xf0)|0x02; /*显示关灯位图*/
                    }
                    else
                    {
                        menu = (menu&0xf0)|0x01; /*显示开灯位图*/
                    }
                }
                else /*其他菜单进入则显示菜单内容*/
                {
                    menu = (menu&0xf0)|0x01;
                }
            }
            /* 菜单选择：低4位不为0时，退回一级菜单 */
            else
            {
                OLED_CLS(); 
                DWT_DelayMs(200);
        
                menu_show_flag = 1;
                menu = menu&0xf0;
            }
            enter_flag = 0;
        }

        /* 按键1和按键2按下判断是否切换 */
        if(left_shift_flag == 1 || right_shift_flag == 1)
        {
            /* 通过低4位判断是否显示的是内容还是主页，为0表示显示主页 */
            if((menu&0x0f)==0)
            {
                if(left_shift_flag)
                {
                    if(menu == 0x00)
                    {
                        /* 第一菜单再向左切换，切换到第四菜单 */
                        menu = 0x40;
                    }
                    else
                    {
                        /* 其他菜单向左切换，切换到上一级菜单 */
                        menu = menu-0x10;
                    }
                }
                if(right_shift_flag)
                {
                    if(menu == 0x40)
                    {
                        /* 第四菜单再向右切换，切换到第一菜单 */
                        menu = 0x00;
                    }
                    else
                    {
                        /* 其他菜单向右切换，切换到下一级菜单 */
                        menu = menu+0x10;
                    }
                }
                right_shift_flag = 0;
                left_shift_flag = 0;
                menu_show_flag = 1;
            }
            /* 显示内容 */
            else
            {
                /* 内容页左键按下 */
                if(left_shift_flag)
                {
                    if(menu == 0x31)
                    {
                        /* 如果是关灯，则切换到开灯 */
                        menu = 0x32;
                    }
                    else if(menu == 0x32)
                    {
                        /* 如果是开灯，则切换到关灯 */
                        menu = 0x31;
                    }
                    if(menu == 0x41)
                    {
                        /* 如果是打坐，则增加打坐次数 */
                        wooden_fish_num++;
                        add_flag =1;
                    }
                }
                /* 内容页右键按下 */
                if(right_shift_flag) 
                {
                    if(menu == 0x31)
                    {
                        /* 如果是开灯，则切换到关灯 */
                        menu = 0x32;
                    }
                    else if(menu == 0x32)
                    {
                        /* 如果是关灯，则切换到开灯 */
                        menu = 0x31;
                    }
                    if(menu == 0x41 || menu == 0x42)
                    {
                        /* 如果是打坐，则清零 */
                        wooden_fish_num = 0;
                        add_flag = 0;
                    }
                }
                right_shift_flag = 0;  /* 右键释放 */
                left_shift_flag = 0;   /* 左键释放 */
            }
        }

        /* 是否需要显示主页 */
        if((menu&0x0f) == 0 && menu_show_flag == 1)
        {
            OLED_CLS(); 
            switch(menu)
            { 
                /* 显示主页 */
                case 0x00:
                    OLED_DrawBitMap((128-50)/2-1,1,50,50,(uint8_t*)home);
                    break;
                /* 显示音乐 */
                case 0x10:
                    OLED_DrawBitMap((128-50)/2-1,1,50,50,(uint8_t*)music);
                    OLED_ShowChinese_F16X16(2,0,11);
                    OLED_ShowChinese_F16X16(2,7,12); 
                    break;
                /* 显示温湿度 */
                case 0x20:
                    OLED_DrawBitMap((128-50)/2,1,50,50,(uint8_t*)humidity);
                    OLED_ShowChinese_F16X16(2,0,11);
                    OLED_ShowChinese_F16X16(2,7,12); 
                    break;
                /* 控制灯具 */
                case 0x30:
                    OLED_DrawBitMap((128-50)/2-1,1,50,50,(uint8_t*)lighting_control);
                    OLED_ShowChinese_F16X16(2,0,11);
                    OLED_ShowChinese_F16X16(2,7,12); 
                    break;
                /* 打坐 */
                case 0x40:
                    OLED_DrawBitMap((128-50)/2-1,1,50,50,(uint8_t*)meditate);
                    OLED_ShowChinese_F16X16(2,0,11);
                    OLED_ShowChinese_F16X16(2,7,12);             
                default:
                    break;
            }
            menu_show_flag = 0;
        }
        /* 显示内容 */
        else
        {
            switch(menu&0xf0)
            { 
                /* 显示主页 */
                case 0x00:
                    OLED_DrawBitMap((128-50)/2-1,1,50,50,(uint8_t*)home);
                    break;
                    
                /* 显示音乐 */
                case 0x10:
                    if(menu ==0x11)
                    {
                        OLED_DrawBitMap(0,0,128,64,(uint8_t*)music1);
                    }
                    break;

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

                /* 控制灯具 */
                case 0x30:
                    if(menu == 0x31)
                    {
                        OLED_DrawBitMap((128-52)/2-1,(64-20)/2,52,20,(uint8_t*)led_off_bmp);
                        led_state_flag = 0;
                        LED_OFF(LED4_GPIO_PORT,LED4_GPIO_PIN,LED_LOW_TRIGGER);
                    }
                    else if(menu == 0x32)
                    {
                        OLED_DrawBitMap((128-52)/2-1,(64-20)/2,52,20 ,(uint8_t*)led_on_bmp);
                        led_state_flag = 1;
                        LED_ON(LED4_GPIO_PORT,LED4_GPIO_PIN,LED_LOW_TRIGGER);
                    }                       
                    break;

                /* 打坐 */
                case 0x40:
                    if(menu == 0x41 && add_flag == 1)
                    {
                        sprintf(str_temp1, "%d",wooden_fish_num);
                        OLED_DrawBitMap((128-85)/2-1,64-50,85,50,(uint8_t*)wooden_fish_on);
                        vTaskDelay(pdMS_TO_TICKS(200));
                        OLED_CLS();
                        OLED_SetPos(0,64);
                        OLED_ShowString_F8X16(0,7,(uint8_t*)str_temp1);
                        OLED_DrawBitMap((128-67)/2-1,64-50,67,50,(uint8_t*)wooden_fish_off);
                        add_flag = 0;
                    }
                    else if(menu == 0x41)
                    {
                        sprintf(str_temp1, "%d",wooden_fish_num);
                        OLED_ShowString_F8X16(0,7,(uint8_t*)str_temp1);
                        OLED_DrawBitMap((128-67)/2-1,64-50,67,50,(uint8_t*)wooden_fish_off);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/*****************************END OF FILE***************************************/


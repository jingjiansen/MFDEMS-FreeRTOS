/**
  ******************************************************************************
  * @file       usart_com.c
  * @author     embedfire
  * @version     V1.0
  * @date        2024
  * @brief      重定向C库printf函数到usart端口
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

#include "usart/bsp_usart.h"
#include "usart/usart_com.h"
#include <stdio.h>

/**
  * @brief  发送一个字节函数
  * @param  pusartx：USARTx(x=1,2,3)/UARTx(x=4,5) 
  * @param  ch:要发送的数据
  * @note   无
  * @retval 无
  */
void USART_SendByte(USART_TypeDef *pusartx,uint8_t ch)
{
    /* 等待发送完成 */
    while(USART_GetFlagStatus(pusartx,USART_FLAG_TC) == RESET);
    
    /* 发送一个字节数据到 pusartx */
    USART_SendData(pusartx,ch);
    
    /* 等待发送数据寄存器为空 */
    while(USART_GetFlagStatus(pusartx,USART_FLAG_TXE) == RESET);
}

/**
  * @brief  发送8位的数组函数
  * @param  pusartx：USARTx(x=1,2,3)/UARTx(x=4,5) 
  * @param  array:要发送的数组
  * @param  num:数组大小.
  * @note   无
  * @retval 无
  */
void USART_SendArray(USART_TypeDef *pusartx,uint8_t *array,uint32_t num)
{
    /* 等待发送完成 */
    while(USART_GetFlagStatus(pusartx,USART_FLAG_TC) == RESET);
    
    for(uint32_t i = 0;i<num;i++)
    {
        /* 发送一个字节数据到 pusartx */
        USART_SendData(pusartx,array[i]);
        
        /* 等待发送数据寄存器为空 */
        while(USART_GetFlagStatus(pusartx,USART_FLAG_TXE) == RESET);
    }
}

/**
  * @brief  发送字符串函数
  * @param  pusartx：USARTx(x=1,2,3)/UARTx(x=4,5) 
  * @param  str:要发送的字符串
  * @note   无
  * @retval 无
  */
void USART_SendString(USART_TypeDef *pusartx,uint8_t *str)
{
    int k = 0;
    /* 等待发送完成 */
    while(USART_GetFlagStatus(pusartx,USART_FLAG_TC) == RESET);
    
    do
    {
        /* 发送一个字节数据到 pusartx */
        USART_SendData(pusartx,*(str+k));
        
        /* 等待发送数据寄存器为空 */
        while(USART_GetFlagStatus(pusartx,USART_FLAG_TXE) == RESET);
        k++;

    } while (*(str+k)!='\0');

}

/**
 * @brief 将一个字符写入到文件中，重定向C库函数printf到串口，重定向后可使用printf函数打印
 * @param  ch:要发送的字符
 * @param  f:文件指针，这里是USART1
 * @note   
 * @retval 成功，返回该字符
 */
int fputc(int ch, FILE *f)
{
  /* 等待发送完成 */
  while(USART_GetFlagStatus(USART_USARTX, USART_FLAG_TC) == RESET);

  /* 发送一个字节数据到串口 */
  USART_SendData(USART_USARTX, (uint16_t) ch);

  /* 等待发送数据寄存器为空 */
  while(USART_GetFlagStatus(USART_USARTX, USART_FLAG_TXE) == RESET);
  
  return (ch);
}


/*****************************END OF FILE***************************************/

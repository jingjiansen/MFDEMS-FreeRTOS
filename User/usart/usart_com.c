#include "usart/usart_com.h"
#include "debug/bsp_debug.h"


/* 发送1个字节 */
void USARTX_SendByte(USART_TypeDef *pusartx, uint8_t ch)
{
    /* 等待发送完成：确保前一个字节已经完全从移位寄存器发出，防止新数据覆盖正在
       发送的数据，TC=1表示移位寄存器已空 */
    while (USART_GetFlagStatus(pusartx, USART_FLAG_TC) == RESET);
    
    /* 写入数据到数据寄存器，数据从DR寄存器自动加载到移位寄存器 */
    USART_SendData(pusartx,ch);
    
    /* 等待TXT标志：确保数据已从DR寄存器转移到移位寄存器，表示可以写入下一字节 */
    while (USART_GetFlagStatus(pusartx, USART_FLAG_TXE) == RESET);
}


/* 发送指定数量字节 */
void USARTX_SendArray(USART_TypeDef *pusartx, uint8_t *array, uint32_t num)
{
    while (USART_GetFlagStatus(pusartx, USART_FLAG_TC) == RESET);
    
    for (uint32_t i = 0; i < num; i++)
    {
        USART_SendData(pusartx,array[i]);
        while (USART_GetFlagStatus(pusartx, USART_FLAG_TXE) == RESET);
    }
}


/* 发送字符串 */
void USARTX_SendString(USART_TypeDef *pusartx, char *str)
{
    uint32_t k = 0;    
    while (USART_GetFlagStatus(pusartx, USART_FLAG_TC) == RESET);
    
    do
    {
        USART_SendData(pusartx,*(str + k));
        k++;
        while (USART_GetFlagStatus(pusartx, USART_FLAG_TXE) == RESET);        
    } while (*(str + k) != '\0');
}


/**
 * @brief 将一个字符写入到文件中,重定向c库函数printf到串口，重定向后可使用printf函数
 * @param ch: 要写入的字符
 * @param f: 指向FILE结构的指针
 * @retval 成功，返回该字符
 */
int fputc(int ch, FILE *f)
{
    /* 等待发送完成 */
    while (USART_GetFlagStatus(DEBUG_USARTX, USART_FLAG_TC) == RESET);
    
    /* 发送一个字节数据到串口 */
    USART_SendData(DEBUG_USARTX, (uint8_t)ch);
    
    /* 等待发送数据寄存器为空 */
    while (USART_GetFlagStatus(DEBUG_USARTX, USART_FLAG_TXE) == RESET);

    return (ch);
}

/*****************************END OF FILE***************************************/


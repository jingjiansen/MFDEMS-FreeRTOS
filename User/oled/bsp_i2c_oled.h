#ifndef __BSP_I2C_OLED_H
#define	__BSP_I2C_OLED_H

#include "stm32f10x.h"

#define OLED_SLAVER_ARRD   (0x3C) /* OLED屏的地址 */

#define OLED_WR_CMD        (0x00) /* 指令模式 */
#define OLED_WR_DATA       (0x40) /* 数据模式 */

#define OLED_ARRAY_SIZE(A) sizeof(A)/sizeof(A[0])  
     
#define TEXTSIZE_F6X8       6  /* 6x8字体 */
#define TEXTSIZE_F8X16      8  /* 8x16字体 */
#define TEXTSIZE_F16X16     16 /* 16x16字体 */

void OLED_Init(void);
ErrorStatus OLED_CheckDevice(uint8_t slave_addr);
ErrorStatus OLED_WriteByte(uint8_t cmd,uint8_t byte);
ErrorStatus OLED_WriteBuffer(uint8_t cmd,uint8_t* buffer,uint32_t num);
void OLED_SetPos(uint8_t y,uint8_t x);
void OLED_SetPos(uint8_t y,uint8_t x);
void OLED_Fill(uint8_t fill_data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);

void OLED_ShowChinese(uint8_t y,uint8_t x,uint8_t n,uint8_t *data_cn);
void OLED_ShowChinese_F16X16(uint8_t line,uint8_t offset,uint8_t n);
void OLED_ShowChar(uint8_t y, uint8_t x, uint8_t char_data,uint8_t textsize);
void OLED_ShowString(uint8_t y, uint8_t x, uint8_t *string_data,uint8_t textsize);
void OLED_ShowString_F8X16(uint8_t line, uint8_t offset, uint8_t *string_data);

void OLED_DrawBitMap(uint8_t x,uint8_t y,uint8_t x_length,uint8_t y_length,uint8_t *raw);

#endif /* __BSP_I2C_OLED_H  */
                                        

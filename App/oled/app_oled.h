#ifndef __APP_OLED_H
#define	__APP_OLED_H

#include "stm32f10x.h"//或#include "stdint.h"

extern uint8_t menu_show_flag;
extern uint8_t left_shift_flag;
extern uint8_t right_shift_flag;
extern uint8_t enter_flag;

void Bsp_Init(void);
void App_Init(void);
void Display_Task(void);
#endif /* __APP_OLED_H  */

#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);  // OLED初始化函数声明
void OLED_Clear(void); // OLED清屏函数声明
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char); // OLED显示一个字符函数声明
void OLED_ShowString(uint8_t Line, uint8_t Column, volatile char *String); // OLED显示字符串函数声明
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);// OLED显示数字（十进制，正数）函数声明
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);// OLED显示数字（十进制，带符号数）函数声明
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);// OLED显示数字（十六进制，正数）函数声明
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);// OLED显示数字（二进制，正数）函数声明

#endif

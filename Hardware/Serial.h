#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>           // 标准输入输出库，用于格式化字符串
#include "stm32f10x.h"       // STM32F10x系列头文件，包含寄存器定义和外设库函数
#include "FreeRTOS.h"        // FreeRTOS操作系统头文件，包含任务管理和调度函数
#include "task.h"            // FreeRTOS任务管理头文件，包含任务创建和删除函数

extern uint8_t Serial_RxFlag; //串口接收标志位，0表示没有接收到数据包，1表示接收到数据包
static uint16_t Serial_Data;  //接收数据变量，存储从ESP32-01S模块接收到的数据，供taskC使用
extern char Serial_RxPacket[]; //定义接收数据包数组，数据包格式"@MSG\r\n"，供taskD使用

void Serial_Init(void);       //串口初始化函数声明
void Serial_SendByte(uint8_t Byte);//串口发送一个字节函数声明
void Serial_SendArray(uint8_t *Array, uint16_t Length);//串口发送一个数组函数声明
void Serial_SendString(char *String);//串口发送一个字符串函数声明
void Serial_SendNumber(uint32_t Number, uint8_t Length);//串口发送一个数字函数声明，Number为要发送的数字，Length为数字的位数
void Serial_Printf(char *format, ...);//串口发送一个格式化字符串函数声明，参数与printf函数相同
void show_RxPacket(void);//显示接受到的以O开头的数据包
void Serial_ESP32_AT(char *AT, char *AT_Show);//发送AT指令并且显示接受的数据
void Serial_ESP32_SendString(char *String);//ESP32服务端向客户端发送数据包 
void Serial_ESP32_SendData(uint16_t data);//ESP32服务端向客户端发送数据
void Serial_ESP32_SendData_Time(uint16_t data);//ESP32客户端定期向服务端发送传感器数据
void ESP32_Init(void);//ESP32模块初始化函数

#endif

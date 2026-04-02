#ifndef __CREATE_TASK_H
#define __CREATE_TASK_H

#include "stm32f10x.h" // STM32F10x系列头文件
#include "FreeRTOS.h"  // FreeRTOS操作系统
#include "task.h"      // FreeRTOS任务管理
#include "LED.h"       // LED操作函数头文件
#include "AD.h"        // AD操作函数头文件
#include "Serial.h"    // 串口操作函数头文件
#include "OLED.h"      // OLED操作函数头文件
#include "queue.h"     // FreeRTOS队列管理
#include "semphr.h"    // FreeRTOS信号量管理
#include <string.h>    // 字符串函数
#include "Timer.h"     // 定时器操作函数头文件
#include "portmacro.h" // FreeRTOS端口相关定义

static TaskHandle_t Task_A_Handle = NULL;// 任务A句柄
static TaskHandle_t Task_B_Handle = NULL;// 任务B句柄
static TaskHandle_t Task_C_Handle = NULL;// 任务C句柄
static TaskHandle_t Task_D_Handle = NULL;// 任务D句柄
static uint32_t i = 0;                   // 定时器计数变量，用于控制发送数据的频率

static BaseType_t xstatus = 0; //任务创建状态
static BaseType_t result = 0;  //任务创建结果

static uint32_t flag;//0表示任务A没有执行结束，1表示任务A执行结束

static SemaphoreHandle_t xOLEDMutex = NULL; // OLED显示资源互斥锁句柄

void BSP_Init(void); //硬件初始化函数声明

void Task_A(void *p);   //任务A函数声明
void Task_B(void *p);   //任务B函数声明
void Task_C(void *p);   //任务C函数声明
void Task_D(void *p);   //任务D函数声明

static QueueHandle_t xQueue_BC = NULL;   // B 到 C 的消息队列句柄

void System_Start(void); //系统开始函数声明

#endif /* __CREATE_TASK_H */

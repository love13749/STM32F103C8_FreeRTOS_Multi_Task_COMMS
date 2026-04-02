#include "stm32f10x.h"  // Device header
#include "CreateTask.h" // 任务创建头文件
#include "Delay.h"      // 延时函数头文件

int main(void)          // 加载操作系统的入口函数
{
  OLED_Init();          // 初始化OLED显示屏
  OLED_ShowString(2, 1, "System Start..."); // 在OLED上显示系统启动信息
  Delay_ms(5000);       // 延时1秒，等待用户看到启动信息，这个时候操作系统还没有启动，任务也没有创建，所以可以使用普通的延时函数
	OLED_Clear();         // 清屏，为后续任务显示做好准备
  System_Start();       // 启动FreeRTOS系统，创建任务并开始调度

  return 0;
}

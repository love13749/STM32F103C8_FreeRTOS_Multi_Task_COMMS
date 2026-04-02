#include "CreateTask.h"

/*
	任务执行流程：
	首先taskA: 负责系统初始化，包括串口、AD模块和ESP32-01S模块的初始化，
	并且taskA在执行结束之后taskB、taskC和taskD才能执行，保证了系统的稳定
	性和可靠性，taskB的执行需要发送数据给ESP32-01S模块，taskC的执行需要
    从消息队列接收数据，taskD的执行需要接收串口发送的数据包，taskD需要接受
	数据包接受状态位Serial_RxFlag的值为1，说明接收到了数据包，才能执行接收
	数据包的相关操作。
*/
void BSP_Init(void)
{
  	flag = 0;         // 初始化标志位，表示任务A没有执行结束
 	Serial_RxFlag = 0;// 初始化串口接收标志位
    LED_Init();       // 初始化LED灯
  	xQueue_BC = xQueueCreate(5, sizeof(uint32_t));// 创建B到C的消息队列，长度为10，每个元素大小为uint32_t
  	configASSERT(xQueue_BC);                      // 断言检查，失败则停在这里

  	xOLEDMutex = xSemaphoreCreateMutex();// 创建OLED显示资源互斥锁
  	configASSERT(xOLEDMutex);            // 断言检查，失败则停在这里
}

/*
    taskA: 负责系统初始化，包括串口、AD模块和ESP32-01S模块的初始化
*/
void Task_A(void *p)
{
  	Serial_Init();              // 初始化串口模块		   
  	AD_Init();                  // 初始化AD模块       
  	ESP32_Init();               // 初始化ESP32-01S模块
  	flag = 1;                   // 任务A执行结束    

  	vTaskDelete(Task_A_Handle); // 当处于这个状态时说明ESP32-01S初始化完成，删除自己
}


/*
    taskB: 负责定时读取AD值，并将AD值发送给ESP32-01S模块，同时在OLED上显示提示信息
*/
void Task_B(void *p)
{
	while(flag ==0) vTaskDelay(pdMS_TO_TICKS(1)); //如果任务A没有执行结束，说明ESP32-01S模块还没有准备好，taskB主动让出CPU，等待任务A执行完成
  	Timer_Init(); //如果任务A执行结束，说明ESP32-01S模块已经准备好，初始化定时器，开启定时器中断，定时器中断函数TIM2_IRQHandler会被调用
	for(;;)
  	{
		xQueueSend(xQueue_BC, &AD_Value[0], 0);    //将AD值发送到消息队列，供taskC使用
		
		vTaskDelay(pdMS_TO_TICKS(1));             // 主动让出 CPU 1ms，阻塞任务B，等待下一次发送机会
 	}
}


/*
    taskC: 负责接收ESP32-01S模块发送的数据，并在OLED上显示
*/
void Task_C(void *p)
{
	while(flag == 0) vTaskDelay(pdMS_TO_TICKS(1)); //如果任务A没有执行结束，说明ESP32-01S模块还没有准备好，taskC主动让出CPU，等待任务A执行完成
	while(1)
	{
	    result = xQueueReceive(xQueue_BC, (void*)&Serial_Data, 0);//从消息队列接收数据，存储在Serial_Data变量中，等待时间为无限长
		if(result != pdPASS) //如果接收失败，说明消息队列中没有数据，taskC主动让出CPU，等待下一次接收机会
		{
			vTaskDelay(pdMS_TO_TICKS(1));             // 主动让出 CPU 1ms
			continue;
		}

		if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)//成功获取到OLED显示资源的互斥锁，说明可以安全地访问OLED显示资源，taskC在OLED上显示接收到的AD值，并释放互斥锁
		{
			OLED_ShowNum(2, 1, Serial_Data, 4); // 在OLED上显示接收到的AD值，显示位置为第1行第1列，长度为4位
			xSemaphoreGive(xOLEDMutex);                   // 释放互斥锁
		}

		vTaskDelay(pdMS_TO_TICKS(1));   // 主动让出 CPU 1ms，阻塞任务C，等待下一次接收机会
	}
}

/*
    taskD: 负责根据接收的ESP32-01S数据控制LED灯，并在OLED上显示接收到的指令
*/
void Task_D(void *p)
{
	while(flag == 0) vTaskDelay(pdMS_TO_TICKS(1)); //如果任务A没有执行结束，说明ESP32-01S模块还没有准备好，taskD主动让出CPU，等待任务A执行完成
	while(1)
	{
        if(Serial_RxFlag)//接收到串口发送的指令数据
		{
			if(strcmp(Serial_RxPacket, "LED1_ON") == 0)//接受到LED1亮的指令
			{
				LED1_ON();
				if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
        		{
            		OLED_ShowString(3, 1, Serial_RxPacket);
            		xSemaphoreGive(xOLEDMutex);
       			}
			}
			else if(strcmp(Serial_RxPacket, "LED1_OFF") == 0)//接收到LED1灭的指令
			{
				LED1_OFF();
				if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
                {
                    OLED_ShowString(3, 1, Serial_RxPacket);
                    xSemaphoreGive(xOLEDMutex);
                }
			}
			else if(strcmp(Serial_RxPacket, "LED2_ON") == 0)//接受到LED2亮的指令
			{
				LED2_ON();
				if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
                {
                    OLED_ShowString(3, 1, Serial_RxPacket);
                    xSemaphoreGive(xOLEDMutex);
                }
			}
			else if(strcmp(Serial_RxPacket, "LED2_OFF") == 0)//接受到LED2灭的指令
			{
				LED2_OFF();
				if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
                {
                    OLED_ShowString(3, 1, Serial_RxPacket);
                    xSemaphoreGive(xOLEDMutex);
                }
			}
			else
			{
				Serial_RxFlag = 0;//如果接收到的数据包不符合以上指令格式，则将数据包位置归零，丢弃这个数据包
			}
			Serial_RxFlag = 0;//初始化接收标志位
		}

		vTaskDelay(pdMS_TO_TICKS(1));   // 主动让出 CPU 1ms，阻塞任务D，等待下一次接收机会
	}
}

/*
	定时中断函数,当定时器TIM2的更新中断发生时，TIM2_IRQHandler函数会被调用，在这个函数中将接收到的AD值发送给ESP32-01S模块
*/
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)		//检查TIM2的更新中断标志位是否被置位
	{
		Serial_ESP32_SendData_Time(AD_Value[0]);            //将接收到的AD值发送给ESP32-01S模块
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);			//清除TIM2的更新中断标志位
	}
}

/*
	System_Start: 负责系统的启动，包括硬件初始化、任务创建和任务调度
*/

void System_Start(void)
{
  	BSP_Init(); // 硬件初始化
	
  	xstatus = xTaskCreate(Task_A, "Task_A", 128, NULL, 2, &Task_A_Handle); // 创建任务A，优先级为2
  	configASSERT(xstatus == pdPASS); // 断言检查，失败则停在
  	xstatus = xTaskCreate(Task_B, "Task_B", 256, NULL, 1, &Task_B_Handle); // 创建任务B，优先级为2
  	configASSERT(xstatus == pdPASS); // 断言检查，失败则停在
  	xstatus = xTaskCreate(Task_C, "Task_C", 128, NULL, 1, &Task_C_Handle); // 创建任务C，优先级为2
  	configASSERT(xstatus == pdPASS); // 断言检查，失败则停在
  	xstatus = xTaskCreate(Task_D, "Task_D", 128, NULL, 1, &Task_D_Handle); // 创建任务D，优先级为2
  	configASSERT(xstatus == pdPASS); // 断言检查，失败则停在

  	vTaskStartScheduler(); // 启动任务调度器
}

/*
	操作系统延时函数的实现原理：
	当调用vTaskDelay函数时，当前任务会被阻塞一段时间，直到指定的时间到达
	或者有其他任务唤醒它。在这段时间内，CPU可以执行其他任务，提高系统的效
	率和响应速度。在vTaskDelay函数内部，FreeRTOS会将当前任务的状态设置为
	阻塞状态，并将它加入到一个等待队列中。然后，FreeRTOS会切换到其他任务继
	续执行。当指定的时间到达时，FreeRTOS会将当前任务从等待队列中移除，并将
	它的状态设置为就绪状态，等待下一次调度机会。需要注意的是，vTaskDelay函
	数的参数是以系统节拍为单位的，而不是以毫秒为单位的。因此，在调用vTaskDelay
	函数时，需要使用pdMS_TO_TICKS宏将毫秒转换为系统节拍数，以确保正确的延时效果。
*/

/*
	任务调度器的实现原理：
	FreeRTOS的任务调度器是基于优先级的抢占式调度算法。当有多个任务处于就绪
	状态时，调度器会选择优先级最高的任务来执行。如果有多个任务具有相同的优先
	级，调度器会按照时间片轮转的方式来分配CPU时间。调度器会在每次系统节拍中
	断时进行调度检查，以确保高优先级的任务能够及时得到执行。当一个任务被阻塞
	或者进入等待状态时，调度器会将它从就绪队列中移除，并将CPU分配给其他就绪
	的任务。当一个任务被唤醒或者进入就绪状态时，调度器会将它加入到就绪队列中，
	并根据优先级进行排序，以确保高优先级的任务能够优先得到执行。
*/

/*
	任务间通信的实现原理：
	在这个系统中，任务B和任务C通过一个消息队列来进行通信。任务B负责定时读取
	AD值，并将AD值发送到消息队列中，供任务C使用。任务C负责从消息队列中接收
	数据，并在OLED上显示。消息队列是一种线程安全的数据结构，可以在多个任务之间
	安全地传递数据。当任务B调用xQueueSend函数将数据发送到消息队列时，FreeRTOS会
	将数据复制到消息队列的内部缓冲区中，并将等待接收数据的任务（如果有）唤醒。当
	任务C调用xQueueReceive函数从消息队列中接收数据时，FreeRTOS会将数据从消息队列的
	内部缓冲区中复制到任务C提供的缓冲区中，并将等待发送数据的任务（如果有）唤醒。
	需要注意的是，消息队列的长度和每个元素的大小需要根据实际需求进行合理设置，以
	确保系统的稳定性和性能。
*/

/*
	任务优先级的设置原则：
	在这个系统中，任务A的优先级设置为2，任务B、任务C和任务D的优先级设置为1。
	这是因为任务A负责系统的初始化工作，包括串口、AD模块和ESP32-01S模块的初始化，
	这些操作需要在其他任务执行之前完成，以确保系统的稳定性和可靠性。任务B、任务C
	和任务D负责系统的正常运行，可以在任务A执行完成之后执行，因此它们的优先级设置为1。
	需要注意的是，优先级的设置需要根据实际需求进行合理调整，以确保系统的响应速度和
	效率。
*/

/*
	任务删除的实现原理：
	在这个系统中，任务A在执行完成之后会调用vTaskDelete函数来删除自己。这是因为任务A
	负责系统的初始化工作，在完成初始化之后就没有继续执行的必要了。通过调用vTaskDelete
	函数，任务A可以释放它占用的系统资源，并将自己从调度器中移除，以节省系统资源和提高
	系统效率。需要注意的是，只有当一个任务不再需要执行时才应该删除它，以避免不必要的
	资源浪费和系统不稳定。
*/

/*
	操作系统资源共享的实现原理：
	在这个系统中，任务B和任务C通过一个消息队列来共享AD值。任务B负责定时读取AD值，并
	将AD值发送到消息队列中，供任务C使用。任务C负责从消息队列中接收数据，并在OLED上显
	示。通过使用消息队列，任务B和任务C可以安全地共享数据，而不需要担心数据竞争和资源
	冲突的问题。消息队列提供了一种线程安全的机制，可以在多个任务之间传递数据，而不需
	要使用复杂的同步机制，如互斥锁或信号量。这种资源共享的方式可以提高系统的效率和响
	应速度，同时也可以简化任务之间的通信和数据管理。
*/

/*
	任务间同步的实现原理：
	在这个系统中，任务A、任务B、任务C和任务D通过一个标志位flag来进行同步。任务A负责
	系统的初始化工作，在完成初始化之后将flag设置为1，表示任务A执行结束。任务B、任务C
	和任务D在执行之前会检查flag的值，如果flag为0，说明任务A还没有执行结束，它们会主
	动让出CPU，等待任务A执行完成后再继续执行。通过使用这个标志位，任务B、任务C和任
	务D可以确保在任务A完成初始化工作之后才开始执行，从而保证了系统的稳定性和可靠性。
	需要注意的是，这种同步方式适用于简单的场景，对于复杂的同步需求，可能需要使用更
	加高级的同步机制，如信号量或事件组。
*/

/*
	操作系统中断处理的实现原理：
	在这个系统中，USART1_IRQHandler函数是一个中断服务函数，用于处理USART1的接收中断
	。当USART1接收到数据时，会触发一个中断，CPU会暂停当前正在执行的任务，转而执行USART1_IRQHandler
	函数。在这个函数中，通过状态机的方式来处理接收到的数据包，确保数据包的完整性和正确
	性。处理完成后，函数会清除中断标志位，并返回到之前被中断的任务继续执行。需要注意的是
	，中断服务函数应该尽量简短和高效，以减少对系统性能的影响，同时也要确保正确地处理和清
	除中断标志位，以避免重复触发同一中断。
*/

/*
	操作系统的互斥和信号量的实现原理：
	在这个系统中，并没有使用互斥锁或信号量来进行任务间的同步和资源共享，而是通过标志位和
	消息队列来实现的。标志位用于任务A、任务B、任务C和任务D之间的同步，确保任务A完成初始
	化工作之后才开始执行其他任务。消息队列用于任务B和任务C之间的数据共享，确保数据的安全
	传递和管理。虽然互斥锁和信号量是常用的同步机制，但在这个系统中，由于任务之间的关系比
	较简单，使用标志位和消息队列已经能够满足需求，并且可以简化代码和提高系统效率。当然，
	在更复杂的系统中，可能需要使用互斥锁或信号量来处理更复杂的同步和资源共享问题。	
*/

/*
	操作系统的内存管理的实现原理：
	在这个系统中，并没有使用动态内存分配的方式来管理内存，而是通过静态分配的方式来管理内
	存。任务A、任务B、任务C和任务D的堆栈空间都是在编译时静态分配的，确保了系统的稳定性和
	可靠性。同时，消息队列也是在编译时静态分配的，确保了系统的效率和性能。虽然动态内存分
	配可以提供更灵活的内存管理方式，但在嵌入式系统中，由于资源有限，使用静态分配的方式可
	以更好地控制内存使用和避免内存泄漏等问题。当然，在更复杂的系统中，可能需要使用动态内
	存分配来处理更复杂的内存管理问题。
*/

/*
	OLED显示数据出现错误，原因在于多任务共享OLED显示资源时，可能会出现数据竞争和资源冲突
	的问题。解决这个问题的方法是使用互斥锁来保护OLED显示资源，确保同一时间只有一个任务能
	够访问OLED显示资源，从而避免数据竞争和资源冲突的问题。具体实现方法是在每个任务中，在
	访问OLED显示资源之前，先获取互斥锁，访问完成后再释放互斥锁，这样就可以保证OLED显示资
	源的安全访问，避免数据错误的发生。
*/

/*
   互斥锁的使用方法：
   在FreeRTOS中，互斥锁是一种特殊的信号量，用于保护共享资源，确保同一时间只有一个任务能
   够访问该资源。使用互斥锁的步骤如下：
1. 创建互斥锁：使用xSemaphoreCreateMutex函数创建一个互斥锁，并获取其句柄。
2. 获取互斥锁：在需要访问共享资源的任务中，使用xSemaphoreTake函数获取互斥锁，参数为
   互斥锁句柄和等待时间。如果成功获取到互斥锁，函数返回pdTRUE；如果等待时间到达仍未获取到
   互斥锁，函数返回pdFALSE。
3. 访问共享资源：在成功获取到互斥锁后，任务可以安全地访问共享资源，进行相关操作。
4. 释放互斥锁：在访问完成后，使用xSemaphoreGive函数释放互斥锁，参数为互斥锁句柄。这样其他
   任务就可以获取到互斥锁，访问共享资源了。需要注意的是，在使用互斥锁时，应该避免死锁的情况
   发生，即多个任务互相等待对方释放互斥锁，导致系统无法继续执行。因此，在设计任务和使用互斥
   锁时，应该合理安排任务的优先级和访问共享资源的顺序，以避免死锁的发生。
*/

/*
   栈高水位标记的使用方法：
   在FreeRTOS中，栈高水位标记是一种用于监测任务堆栈使用情况的工具，可以帮助开发者了解任务
   的堆栈使用情况，避免堆栈溢出等问题。使用栈高水位标记的步骤如下：
1. 定义变量：在任务函数中定义一个变量，用于存储栈高水位标记的值，通常使用uxHighWaterMark_TaskX
   的命名方式，其中X表示任务的名称。
2. 获取栈高水位标记：在任务函数中，使用uxTaskGetStackHighWaterMark函数获取当前任务的栈高
   水位标记，参数为NULL，表示获取当前任务的栈高水位标记。函数返回一个无符号整数，表示当前任
   务的堆栈剩余空间大小。
3. 显示栈高水位标记：可以将获取到的栈高水位标记显示在OLED上，或者通过串口输出，以便开发者了
   解任务的堆栈使用情况。需要注意的是，栈高水位标记的值越小，表示任务的堆栈使用越多，可能存
   在堆栈溢出的风险。因此，在开发过程中，应该定期监测任务的栈高水位标记，确保任务的堆栈使用
   在安全范围内，避免出现堆栈溢出等问题。
*/

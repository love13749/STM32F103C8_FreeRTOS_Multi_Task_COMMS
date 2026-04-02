#include "stm32f10x.h"                  // Device header

/**
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init(void)
{
	/*
    开启时钟
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);            // 开启TIM1的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);            // 开启AFIO时钟（可选，用于重映射）
    
    /*
    GPIO初始化
    */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                  // 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);                          // 将PA8~11引脚初始化为复用推挽输出

    /*
    配置时钟源
    */
    TIM_InternalClockConfig(TIM1);                                  // 选择TIM1为内部时钟
    
    /*
    时基单元初始化
    */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;                 // ARR值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;              // PSC值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（高级定时器特有）
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);             // 配置TIM1的时基单元
    
    /*
    输出比较初始化
    */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);                         // 结构体初始化
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               // PWM模式1
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 输出极性高
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;                              // 初始CCR值
    
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);                        // 配置通道1
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);                        // 配置通道2
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);                        // 配置通道3
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);                        // 配置通道4
    
    /*
    预装载使能
    */
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);               // 通道1预装载使能
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);               // 通道2预装载使能
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);               // 通道3预装载使能
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);               // 通道4预装载使能
    TIM_ARRPreloadConfig(TIM1, ENABLE);                             // ARR预装载使能
    
    /*
    刹车和死区时间配置（高级定时器必须配置）
    */
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    TIM_BDTRStructInit(&TIM_BDTRInitStructure);                     // 初始化结构体
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;     // 运行模式下OC/OCN使能
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;     // 空闲模式下OC/OCN使能
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;          // 锁定级别1
    TIM_BDTRInitStructure.TIM_DeadTime = 0;                         // 死区时间（0-无死区）
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;            // 刹车禁用
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High; // 刹车极性高
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; // 自动输出使能
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);                   // 配置BDTR寄存器
    
    /*
    TIM使能
    */
    TIM_Cmd(TIM1, ENABLE);                                          // 使能TIM1
    
    /*
    高级定时器必须使能主输出
    */
    TIM_CtrlPWMOutputs(TIM1, ENABLE); 
}

/**
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *           占空比Duty = CCR / (ARR + 1)
  */
void PWM_SetCompare3(uint16_t Compare)
{
	TIM_SetCompare4(TIM1, Compare);		//设置CCR3的值
}

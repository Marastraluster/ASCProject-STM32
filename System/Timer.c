#include "stm32f10x.h"                  // Device header

/**
  * 函    数：定时中断初始化
  * 参    数：无
  * 返 回 值：无
  */
void Timer_Init(void)
{
	/*开启时钟*/
	RCC_APB2eriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);				//将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元	
	
	/*中断输出配置*/
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);						//清除定时器更新标志位
																//TIM_TimeBaseInit函数末尾，手动产生了更新事件
																//若不清除此标志位，则开启中断后，会立刻进入一次中断
																//如果不介意此问题，则不清除此标志位也可
	
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);					//开启TIM2的更新中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;				//选择配置NVIC的TIM1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM2，定时器开始运行
	/* 新增：TIM3 1ms 用于按键去抖 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitTypeDef t3;
    t3.TIM_ClockDivision = TIM_CKD_DIV1;
    t3.TIM_CounterMode = TIM_CounterMode_Up;
    t3.TIM_Period = 100 - 1;        // 100 / 100kHz = 1ms
    t3.TIM_Prescaler = 720 - 1;     // 72MHz / 720 = 100kHz
    t3.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &t3);
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic3;
    nvic3.NVIC_IRQChannel = TIM3_IRQn;
    nvic3.NVIC_IRQChannelCmd = ENABLE;
    nvic3.NVIC_IRQChannelPreemptionPriority = 3; // 低于TIM1
    nvic3.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic3);

    TIM_Cmd(TIM3, ENABLE);

	/*添加TIM2和TIM4定时器初始化*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef t2;
    t2.TIM_ClockDivision = TIM_CKD_DIV1;
    t2.TIM_CounterMode = TIM_CounterMode_Up;
    t2.TIM_Period = 100 - 1;        // 100 / 100kHz = 1ms
    t2.TIM_Prescaler = 720 - 1;     // 72MHz / 720 = 100kHz
    t2.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &t2);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic2;
    nvic2.NVIC_IRQChannel = TIM2_IRQn;
    nvic2.NVIC_IRQChannelCmd = ENABLE;
    nvic2.NVIC_IRQChannelPreemptionPriority = 4; // 低于TIM1
    nvic2.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic2);

    TIM_Cmd(TIM4, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseInitTypeDef t4;
    t4.TIM_ClockDivision = TIM_CKD_DIV1;
    t4.TIM_CounterMode = TIM_CounterMode_Up;
    t4.TIM_Period = 100 - 1;        // 100 / 100kHz = 1ms
    t4.TIM_Prescaler = 720 - 1;     // 72MHz / 720 = 100kHz
    t4.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &t4);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic4;
    nvic4.NVIC_IRQChannel = TIM2_IRQn;
    nvic4.NVIC_IRQChannelCmd = ENABLE;
    nvic4.NVIC_IRQChannelPreemptionPriority = 5; // 低于TIM1
    nvic4.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic4);

    TIM_Cmd(TIM4, ENABLE);
}

/* 定时器中断函数，可以复制到使用它的地方
void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
*/
/*
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == sSET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        Key_Scan();  // 1ms 调一次
    }
}
*/
/*
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == sSET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Key_Scan();  // 1ms 调一次
    }
}
*/
/*
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == sSET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        Key_Scan();  // 1ms 调一次
    }
}
*/
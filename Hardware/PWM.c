#include "stm32f10x.h"                  // Device header
#include "PWM.h"

// PWM初始化函数
void PWM_Init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 配置GPIOA的Pin2和Pin3为复用推挽输出（TIM2_CH3和TIM2_CH4）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 定时器时基配置 - 修正为10kHz PWM频率
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;        // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;       // 预分频器 72MHz/72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // PWM模式配置
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比为0
    
    // 初始化通道3和4
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);  // 通道3 - PA2 (PWMA)
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // 通道4 - PA3 (PWMB)
    
    // 使能预装载寄存器
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 使能ARR预装载寄存器
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 设置两个电机的PWM值
void PWM_SetMotorCompare(uint16_t compareL, uint16_t compareR){
    // 限制PWM值在0-1000范围内
    if(compareL > 1000) compareL = 1000;
    if(compareR > 1000) compareR = 1000;
    
    TIM_SetCompare3(TIM2, compareL);  // 左电机PWMA (PA2)
    TIM_SetCompare4(TIM2, compareR);  // 右电机PWMB (PA3)
}
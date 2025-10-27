#include "stm32f10x.h"                  // Device header



void PWM_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef timer;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 10000-1;
    timer.TIM_Prescaler = 7200-1;
    timer.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_OCInitTypeDef outputChannel;
    TIM_OCStructInit(&outputChannel);
    //outputChannel.TIM_OCMode = TIM_OCMode_PWM1;
    //outputChannel.TIM_OutputState = TIM_OutputState_Enable;
    //outputChannel.TIM_Pulse = 5000;
    //outputChannel.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &outputChannel);
    TIM_Cmd(TIM2, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    
}
void PWM_SetCompare(uint16_t compare){
    TIM_SetCompare3(TIM2, compare);
}
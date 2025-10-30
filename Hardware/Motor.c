#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Motor_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob;
    gpiob.GPIO_Mode = GPIO_Mode_Out_PP;
    gpiob.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpiob.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpiob);
    PWM_Init();
}

void Motor_SetSpeed(int16_t speedL, int16_t speedR){
    // 左电机方向控制 (AIN1: PB12, AIN2: PB13)
    if(speedL >= 0){
        // 正转
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    } else {
        // 反转
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        speedL = -speedL;  // 取绝对值
    }
    
    // 右电机方向控制 (BIN1: PB14, BIN2: PB15)
    if(speedR >= 0){
        // 正转
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    } else {
        // 反转
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
        speedR = -speedR;  // 取绝对值
    }
    
    // 限制PWM值在0-1000范围内（新的PWM范围）
    if(speedL > 1000) speedL = 1000;
    if(speedR > 1000) speedR = 1000;
    
    // 设置PWM占空比
    PWM_SetMotorCompare(speedL, speedR);
}
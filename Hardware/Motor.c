#include "stm32f10x.h"                  // Device header
#include "PWM.h"


void Motor_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob;
    gpiob.GPIO_Mode = GPIO_Mode_Out_PP;
    gpiob.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    gpiob.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpiob);
    PWM_Init();
}
void Motor_SetSpeed(int16_t speedL, int16_t speedR){
    if(speedL>=0){
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }else{
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        speedL=-speedL;
    }
    if(speedR>=0){
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    }else{
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
        speedR=-speedR;
    }
    if(speedL>10000) speedL=10000;
    if(speedR>10000) speedR=10000;
    PWM_SetCompare(speedL);
    PWM_SetCompare(speedR);
}
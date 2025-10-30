#include "stm32f10x.h"                  // Device header




void Encoder_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioa;
    gpioa.GPIO_Mode = GPIO_Mode_IPU;
    gpioa.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    gpioa.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioa);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob;
    gpiob.GPIO_Mode = GPIO_Mode_IPU;
    gpiob.GPIO_Pin = GPIO_Pin_6| GPIO_Pin_7;
    gpiob.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpiob);
}
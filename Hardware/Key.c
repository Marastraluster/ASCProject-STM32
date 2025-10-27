#include "stm32f10x.h"                  // Device header





void Key_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioa;
    gpioa.GPIO_Mode = GPIO_Mode_IPU;
    gpioa.GPIO_Pin = GPIO_Pin_0;
    gpioa.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioa);
}

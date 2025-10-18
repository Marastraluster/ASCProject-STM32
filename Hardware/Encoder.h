#ifndef ENCODER_H
#define ENCODER_H
#include "stm32f10x.h"

void Encoder_Init(void);
void Encoder_EXTI_Handler(uint8_t line); // 在 EXTI0/1 IRQ 调用
int  Encoder_FetchDelta(void);           // 读取并清零累计步数（正/负）
void Encoder_Clear(void);

#endif
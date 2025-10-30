#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

// 函数声明
void PWM_Init(void);
void PWM_SetMotorCompare(uint16_t compareL, uint16_t compareR);

#endif
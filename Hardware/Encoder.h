#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

// 编码器结构体
typedef struct {
    TIM_TypeDef* TIMx;          // 定时器
    int32_t total_count;        // 累计编码器计数值（处理溢出）
    int16_t last_count;         // 上次定时器计数值
    float position;             // 当前位置
    float target_position;      // 目标位置
    float kp, ki, kd;          // PID参数
    float last_error;           // 上次误差
    float prev_error;           // 上上次误差
    int16_t output;             // 输出PWM值
} Encoder_PID_t;

// 电机结构体声明
extern Encoder_PID_t motor1, motor2;

// 函数声明
void Encoder_Init(void);
void Encoder_Update(Encoder_PID_t* motor);
float Incremental_PID_Calculate(Encoder_PID_t* motor, float target, float current);
void Motor_Set_PWM(Encoder_PID_t* motor, int16_t pwm);
int32_t Encoder_Get_TotalCount(Encoder_PID_t* motor);
void Encoder_Clear_Count(Encoder_PID_t* motor);

#endif
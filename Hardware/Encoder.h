#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

// 编码器结构体
typedef struct {
    TIM_TypeDef* TIMx;          // 定时器
    int32_t total_count;        // 累计编码器计数值
    int16_t last_count;         // 上次定时器计数值
    float position;             // 当前位置（角度）
    float target_position;      // 目标位置（角度）
    float target_velocity;      // 目标速度
    float current_velocity;     // 当前速度
    
    // 位置式PID参数
    float pos_kp, pos_ki, pos_kd;    // 位置环PID参数
    float pos_integral;              // 位置积分项
    float pos_last_error;            // 位置上次误差
    
    // 速度式PID参数  
    float vel_kp, vel_ki, vel_kd;    // 速度环PID参数
    float vel_last_error;             // 速度上次误差
    float vel_prev_error;             // 速度上上次误差
    
    int16_t output;             // 输出PWM值
    int32_t last_total_count;   // 上次总计数（用于速度计算）
    
    uint8_t follow_mode;        // 0:速度控制, 1:位置跟随
} Encoder_PID_t;

// 电机结构体声明
extern Encoder_PID_t motor1, motor2;

// 函数声明
void Encoder_Init(void);
void Encoder_Update(Encoder_PID_t* motor);
void Encoder_CalculateVelocity(Encoder_PID_t* motor);
float Position_PID_Calculate(Encoder_PID_t* motor, float target, float current);
float Incremental_PID_Calculate(Encoder_PID_t* motor, float target, float current);
int32_t Encoder_Get_TotalCount(Encoder_PID_t* motor);
void Encoder_Clear_Count(Encoder_PID_t* motor);
void SetMotorVelocity(Encoder_PID_t* motor, float velocity);
void SetMotorPosition(Encoder_PID_t* motor, float position);
void EnablePositionFollow(Encoder_PID_t* follower, Encoder_PID_t* leader);
void DisablePositionFollow(Encoder_PID_t* motor);

#endif
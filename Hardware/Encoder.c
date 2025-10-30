#include "stm32f10x.h"                  
#include "encoder.h"
#include <math.h>

// 电机结构体定义
Encoder_PID_t motor1, motor2;

// 位置跟随标志
uint8_t position_follow_enabled = 0;

void Encoder_Init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
    
    // 电机1编码器 - TIM3在PA6,PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 定时器基础配置 - 编码器模式
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 编码器接口配置
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 6;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    // 电机2编码器 - TIM4在PB6,PB7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    
    // 清除计数器并启动定时器
    TIM_SetCounter(TIM3, 0);
    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    
    // 初始化电机1参数（主电机，手动控制）
    motor1.TIMx = TIM3;
    motor1.total_count = 0;
    motor1.last_count = 0;
    motor1.position = 0;
    motor1.target_position = 0;
    motor1.target_velocity = 0;
    motor1.current_velocity = 0;
    
    // 位置PID参数（如果电机1也需要位置控制）
    motor1.pos_kp = 4.0f;
    motor1.pos_ki = 0.1f;
    motor1.pos_kd = 0.5f;
    motor1.pos_integral = 0;
    motor1.pos_last_error = 0;
    
    // 速度PID参数
    motor1.vel_kp = 3.0f;
    motor1.vel_ki = 0.5f;
    motor1.vel_kd = 0.1f;
    motor1.vel_last_error = 0;
    motor1.vel_prev_error = 0;
    
    motor1.output = 0;
    motor1.last_total_count = 0;
    motor1.follow_mode = 0;
    
    // 初始化电机2参数（从电机，跟随电机1）
    motor2.TIMx = TIM4;
    motor2.total_count = 0;
    motor2.last_count = 0;
    motor2.position = 0;
    motor2.target_position = 0;
    motor2.target_velocity = 0;
    motor2.current_velocity = 0;
    
    // 位置PID参数 - 调整这些参数来优化跟随性能
    motor2.pos_kp = 5.0f;      // 比例系数 - 影响响应速度
    motor2.pos_ki = 0.05f;     // 积分系数 - 消除静差
    motor2.pos_kd = 0.8f;      // 微分系数 - 抑制振荡
    motor2.pos_integral = 0;
    motor2.pos_last_error = 0;
    
    // 速度PID参数
    motor2.vel_kp = 3.0f;
    motor2.vel_ki = 0.5f;
    motor2.vel_kd = 0.1f;
    motor2.vel_last_error = 0;
    motor2.vel_prev_error = 0;
    
    motor2.output = 0;
    motor2.last_total_count = 0;
    motor2.follow_mode = 0;
}

// 获取累计编码器计数值
int32_t Encoder_Get_TotalCount(Encoder_PID_t* motor)
{
    return motor->total_count;
}

// 清除编码器计数
void Encoder_Clear_Count(Encoder_PID_t* motor)
{
    TIM_SetCounter(motor->TIMx, 0);
    motor->total_count = 0;
    motor->last_count = 0;
    motor->position = 0;
    motor->last_total_count = 0;
    motor->current_velocity = 0;
    motor->pos_integral = 0;
    motor->pos_last_error = 0;
}

// 更新编码器数据
void Encoder_Update(Encoder_PID_t* motor)
{
    int16_t current_count, count_diff;
    
    // 读取当前定时器计数值
    current_count = (int16_t)TIM_GetCounter(motor->TIMx);
    
    // 计算计数值变化（处理溢出）
    count_diff = current_count - motor->last_count;
    
    if(count_diff > 32767) {
        count_diff -= 65536;
    } else if(count_diff < -32767) {
        count_diff += 65536;
    }
    
    // 累加到总计数
    motor->total_count += count_diff;
    
    // 更新位置（将编码器计数转换为角度，假设1000计数=1圈）
    motor->position = (float)motor->total_count / 1000.0f * 360.0f; // 转换为角度
    
    // 保存当前计数值用于下次计算
    motor->last_count = current_count;
}

// 计算电机速度
void Encoder_CalculateVelocity(Encoder_PID_t* motor)
{
    int32_t count_diff = motor->total_count - motor->last_total_count;
    motor->current_velocity = (float)count_diff;
    motor->last_total_count = motor->total_count;
}

// 位置式PID计算
float Position_PID_Calculate(Encoder_PID_t* motor, float target, float current)
{
    float error, integral, derivative;
    float output;
    
    // 计算当前误差
    error = target - current;
    
    // 积分项（带限幅防止积分饱和）
    motor->pos_integral += error;
    if(motor->pos_integral > 1000) motor->pos_integral = 1000;
    if(motor->pos_integral < -1000) motor->pos_integral = -1000;
    integral = motor->pos_integral;
    
    // 微分项
    derivative = error - motor->pos_last_error;
    
    // 位置式PID公式: u(k) = Kp*e(k) + Ki*∑e(k) + Kd*(e(k)-e(k-1))
    output = motor->pos_kp * error + motor->pos_ki * integral + motor->pos_kd * derivative;
    
    // 更新误差历史
    motor->pos_last_error = error;
    
    return output;
}

// 增量式PID计算（速度环）
float Incremental_PID_Calculate(Encoder_PID_t* motor, float target, float current)
{
    float error, delta_error, delta2_error;
    float delta_output;
    
    error = target - current;
    delta_error = error - motor->vel_last_error;
    delta2_error = error - 2 * motor->vel_last_error + motor->vel_prev_error;
    
    delta_output = motor->vel_kp * delta_error + motor->vel_ki * error + motor->vel_kd * delta2_error;
    
    motor->vel_prev_error = motor->vel_last_error;
    motor->vel_last_error = error;
    
    return delta_output;
}

// 设置电机目标速度
void SetMotorVelocity(Encoder_PID_t* motor, float velocity)
{
    motor->target_velocity = velocity;
    motor->follow_mode = 0; // 速度模式
}

// 设置电机目标位置
void SetMotorPosition(Encoder_PID_t* motor, float position)
{
    motor->target_position = position;
    motor->follow_mode = 0; // 位置模式（非跟随）
}

// 启用位置跟随模式
void EnablePositionFollow(Encoder_PID_t* follower, Encoder_PID_t* leader)
{
    follower->follow_mode = 1;
    // 不清除位置，保持当前位置关系
}

// 禁用位置跟随模式
void DisablePositionFollow(Encoder_PID_t* motor)
{
    motor->follow_mode = 0;
}
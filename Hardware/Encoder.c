#include "stm32f10x.h"                  // Device header
#include "encoder.h"

// 电机结构体定义
Encoder_PID_t motor1, motor2;

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
    
    // 定时器基础配置
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
    
    // 初始化电机参数
    // 电机1使用TIM3
    motor1.TIMx = TIM3;
    motor1.total_count = 0;
    motor1.last_count = 0;
    motor1.position = 0;
    motor1.target_position = 0;
    motor1.kp = 0.5f;    // 初始比例系数
    motor1.ki = 0.01f;   // 积分系数  
    motor1.kd = 0.05f;   // 微分系数
    motor1.last_error = 0;
    motor1.prev_error = 0;
    motor1.output = 0;
    
    // 电机2使用TIM4
    motor2.TIMx = TIM4;
    motor2.total_count = 0;
    motor2.last_count = 0;
    motor2.position = 0;
    motor2.target_position = 0;
    motor2.kp = 0.5f;
    motor2.ki = 0.01f;
    motor2.kd = 0.05f;
    motor2.last_error = 0;
    motor2.prev_error = 0;
    motor2.output = 0;
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
}

// 更新编码器数据（处理溢出）
void Encoder_Update(Encoder_PID_t* motor)
{
    int16_t current_count, count_diff;
    
    // 读取当前定时器计数值
    current_count = (int16_t)TIM_GetCounter(motor->TIMx);
    
    // 计算计数值变化（处理溢出）
    count_diff = current_count - motor->last_count;
    
    // 如果变化量过大，说明发生了溢出
    if(count_diff > 32767) {
        count_diff -= 65536;  // 向下溢出修正
    } else if(count_diff < -32767) {
        count_diff += 65536;  // 向上溢出修正
    }
    
    // 累加到总计数
    motor->total_count += count_diff;
    
    // 更新位置（这里直接用计数值作为位置）
    motor->position = (float)motor->total_count;
    
    // 保存当前计数值用于下次计算
    motor->last_count = current_count;
}

// 增量式PID计算
float Incremental_PID_Calculate(Encoder_PID_t* motor, float target, float current)
{
    float error, delta_error, delta2_error;
    float delta_output;
    
    // 计算当前误差
    error = target - current;
    
    // 计算误差变化量
    delta_error = error - motor->last_error;
    delta2_error = error - 2 * motor->last_error + motor->prev_error;
    
    // 增量式PID公式: Δu = Kp*(e(k)-e(k-1)) + Ki*e(k) + Kd*(e(k)-2e(k-1)+e(k-2))
    delta_output = motor->kp * delta_error + motor->ki * error + motor->kd * delta2_error;
    
    // 更新误差历史
    motor->prev_error = motor->last_error;
    motor->last_error = error;
    
    return delta_output;
}
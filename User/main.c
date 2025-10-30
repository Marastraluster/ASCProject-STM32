#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#include "string.h"
#include "Timer.h"
#include "Delay.h"
#include "encoder.h"

// 全局变量
uint8_t Speed = 0;
uint8_t RxBuffer[100];          // 串口接收缓冲区
uint8_t RxIndex = 0;            // 接收缓冲区索引
uint8_t RxFlag = 0;             // 接收完成标志
char command[100];              // 命令缓冲区

// 系统时间（用于控制周期）
uint32_t system_time = 0;

// 函数声明
void ProcessSerialCommand(char* cmd);
void SendMotorData(void);

int main(void){
    OLED_Init();
    Motor_Init();
    Key_Init();
    Serial_Init();
    Timer_Init();
    
    // 初始化编码器
    Encoder_Init();
    
    // 初始目标速度为0
    SetMotorVelocity(&motor1, 0);
    SetMotorVelocity(&motor2, 0);
    
    Serial_SendString("System Ready\r\n");
    Serial_SendString("Send @speed% xxx to set target speed\r\n");
    
    while (1){
        // 处理串口命令
        if(RxFlag){
            RxFlag = 0;
            ProcessSerialCommand(command);
        }
        
        // 每100ms发送一次电机数据到上位机
        if(system_time % 10 == 0){  // 100ms
            SendMotorData();
        }
        
        // 简单的非阻塞延时
        Delay_ms(1);
    }
}

// TIM1中断处理函数 - 10ms控制周期
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
    {
        float pid_output_L, pid_output_R;
        
        // 更新时间
        system_time++;
        
        // 更新编码器数据
        Encoder_Update(&motor1);
        Encoder_Update(&motor2);
        
        // 计算电机速度
        Encoder_CalculateVelocity(&motor1);
        Encoder_CalculateVelocity(&motor2);
        
        // 速度环PID控制
        pid_output_L = Incremental_PID_Calculate(&motor1, motor1.target_velocity, motor1.current_velocity);
        pid_output_R = Incremental_PID_Calculate(&motor2, motor2.target_velocity, motor2.current_velocity);
        
        // 累加输出
        motor1.output += (int16_t)pid_output_L;
        motor2.output += (int16_t)pid_output_R;
        
        // 限制输出范围
        if(motor1.output > 1000) motor1.output = 1000;
        if(motor1.output < -1000) motor1.output = -1000;
        if(motor2.output > 1000) motor2.output = 1000;
        if(motor2.output < -1000) motor2.output = -1000;
        
        // 设置电机速度
        Motor_SetSpeed(motor1.output, motor2.output);
        
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}

// 处理串口命令
void ProcessSerialCommand(char* cmd)
{
    char* token;
    float speed_value;
    
    // 检查是否是速度设置命令
    if(strstr(cmd, "@speed%") != NULL){
        // 找到数字部分
        token = strtok(cmd, " ");
        token = strtok(NULL, " ");
        
        if(token != NULL){
            speed_value = atof(token);  // 字符串转浮点数
            
            // 设置两个电机的目标速度（方向相反）
            SetMotorVelocity(&motor1, speed_value);
            SetMotorVelocity(&motor2, -speed_value);  // 反向
            
            // 回复确认信息
            Serial_Printf("Set target speed: %.1f (Motor1: %.1f, Motor2: %.1f)\r\n", 
                         speed_value, speed_value, -speed_value);
        }
    }
}

// 发送电机数据到上位机（用于绘图）
void SendMotorData(void)
{
    // 发送格式：目标速度,实际速度1,实际速度2,PWM输出1,PWM输出2
    Serial_Printf("%.1f,%.1f,%.1f,%d,%d\r\n",
                 motor1.target_velocity,
                 motor1.current_velocity,
                 motor2.current_velocity,
                 motor1.output,
                 motor2.output);
}

// 修改串口中断处理函数（在Serial.c中添加或在main.c中重写）
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t received_char = USART_ReceiveData(USART1);
        
        // 处理回车或换行符（命令结束）
        if(received_char == '\r' || received_char == '\n'){
            if(RxIndex > 0){
                command[RxIndex] = '\0';  // 字符串结束符
                RxFlag = 1;               // 设置命令接收完成标志
                RxIndex = 0;              // 重置索引
            }
        }
        // 处理普通字符
        else if(RxIndex < sizeof(RxBuffer)-1){
            RxBuffer[RxIndex] = received_char;
            command[RxIndex] = received_char;
            RxIndex++;
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
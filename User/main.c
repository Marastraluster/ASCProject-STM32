#include "stm32f10x.h"                  
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
uint8_t RxBuffer[100];          
uint8_t RxIndex = 0;            
uint8_t RxFlag = 0;             
char command[100];              

// 系统时间
uint32_t system_time = 0;

// 位置跟随标志
uint8_t position_follow_active = 0;

// 函数声明
void ProcessSerialCommand(char* cmd);
void SendMotorData(void);
void TogglePositionFollow(void);

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
    
    OLED_ShowString(1, 1, "Position Follow");
    OLED_ShowString(2, 1, "Press KEY to toggle");
    OLED_ShowString(3, 1, "Follow: OFF");
    
    Serial_SendString("System Ready\r\n");
    Serial_SendString("Press KEY to toggle position follow\r\n");
    Serial_SendString("Send @pos_follow to enable/disable\r\n");
    
    while (1){
        // 按键处理 - 切换位置跟随模式
        if(Key_GetNum() == 1){
            TogglePositionFollow();
            Delay_ms(300); // 防抖
        }
        
        // 处理串口命令
        if(RxFlag){
            RxFlag = 0;
            ProcessSerialCommand(command);
        }
        
        // 每100ms发送一次电机数据到上位机
        if(system_time % 10 == 0){
            SendMotorData();
            
            // 在OLED上显示位置信息
            OLED_ShowSignedNum(4, 1, (int16_t)motor1.position, 5);
            OLED_ShowString(4, 7, "deg");
            OLED_ShowSignedNum(5, 1, (int16_t)motor2.position, 5);
            OLED_ShowString(5, 7, "deg");
        }
        
        Delay_ms(1);
    }
}

// TIM1中断处理函数 - 10ms控制周期
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
    {
        float pid_output;
        
        // 更新时间
        system_time++;
        
        // 更新编码器数据
        Encoder_Update(&motor1);
        Encoder_Update(&motor2);
        
        // 计算电机速度
        Encoder_CalculateVelocity(&motor1);
        Encoder_CalculateVelocity(&motor2);
        
        if(position_follow_active){
            // 位置跟随模式：电机2跟随电机1的位置
            
            // 设置电机2的目标位置为电机1的当前位置
            motor2.target_position = motor1.position;
            
            // 使用位置式PID控制电机2
            pid_output = Position_PID_Calculate(&motor2, motor2.target_position, motor2.position);
            
            // 限制输出范围
            if(pid_output > 1000) pid_output = 1000;
            if(pid_output < -1000) pid_output = -1000;
            
            motor2.output = (int16_t)pid_output;
            
            // 电机1自由转动（手动控制）
            motor1.output = 0;
        }
        else{
            // 正常速度控制模式
            float pid_output_L, pid_output_R;
            
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
        }
        
        // 设置电机速度
        Motor_SetSpeed(motor1.output, motor2.output);
        
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}

// 切换位置跟随模式
void TogglePositionFollow(void)
{
    position_follow_active = !position_follow_active;
    
    if(position_follow_active){
        // 启用位置跟随
        EnablePositionFollow(&motor2, &motor1);
        OLED_ShowString(3, 1, "Follow: ON ");
        Serial_SendString("Position Follow: ON\r\n");
        Serial_SendString("Rotate Motor1, Motor2 will follow\r\n");
    }
    else{
        // 禁用位置跟随
        DisablePositionFollow(&motor2);
        OLED_ShowString(3, 1, "Follow: OFF");
        Serial_SendString("Position Follow: OFF\r\n");
    }
}

// 处理串口命令
void ProcessSerialCommand(char* cmd)
{
    char* token;
    float value;
    
    // 位置跟随命令
    if(strstr(cmd, "@pos_follow") != NULL){
        TogglePositionFollow();
    }
    // 速度设置命令
    else if(strstr(cmd, "@speed%") != NULL){
        token = strtok(cmd, " ");
        token = strtok(NULL, " ");
        
        if(token != NULL){
            value = atof(token);
            SetMotorVelocity(&motor1, value);
            SetMotorVelocity(&motor2, -value);
            Serial_Printf("Set speed: %.1f\r\n", value);
        }
    }
    // 位置设置命令
    else if(strstr(cmd, "@position") != NULL){
        token = strtok(cmd, " ");
        token = strtok(NULL, " ");
        
        if(token != NULL){
            value = atof(token);
            SetMotorPosition(&motor1, value);
            SetMotorPosition(&motor2, value);
            Serial_Printf("Set position: %.1f deg\r\n", value);
        }
    }
}

// 发送电机数据到上位机
void SendMotorData(void)
{
    if(position_follow_active){
        // 位置跟随模式：发送位置数据
        Serial_Printf("FOLLOW: %.1f,%.1f,%.1f,%d\r\n",
                     motor1.position,           // 电机1实际位置
                     motor2.position,           // 电机2实际位置  
                     motor2.target_position,    // 电机2目标位置（=电机1位置）
                     motor2.output);            // 电机2输出
    }
    else{
        // 速度控制模式：发送速度数据
        Serial_Printf("SPEED: %.1f,%.1f,%.1f,%d,%d\r\n",
                     motor1.target_velocity,
                     motor1.current_velocity,
                     motor2.current_velocity,
                     motor1.output,
                     motor2.output);
    }
}

// 串口中断处理函数
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t received_char = USART_ReceiveData(USART1);
        
        if(received_char == '\r' || received_char == '\n'){
            if(RxIndex > 0){
                command[RxIndex] = '\0';
                RxFlag = 1;
                RxIndex = 0;
            }
        }
        else if(RxIndex < sizeof(RxBuffer)-1){
            RxBuffer[RxIndex] = received_char;
            command[RxIndex] = received_char;
            RxIndex++;
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}